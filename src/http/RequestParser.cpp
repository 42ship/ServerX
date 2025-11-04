#include "http/RequestParser.hpp"
#include "common/filesystem.hpp"
#include "http/Headers.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace http {

RequestParser::RequestParser(Request &req, size_t maxContentSize, size_t maxHeaderSize)
    : maxContentSize_(maxContentSize),
      maxHeaderSize_(maxHeaderSize),
      request_(req),
      startLine_(req.requestLine_),
      headers_(req.headers()) {
    LOG_TRACE("RequestParser::RequestParser(): Initializing...");
    buffer_.reserve(8192);
    reset();
    (void)request_;
}

void RequestParser::reset() {
    LOG_TRACE("RequestParser::reset(): Resetting parser state");
    startLine_.method = RequestStartLine::UNKNOWN;
    state_ = READING_HEADERS;
    contentLength_ = 0;
    contentLength_ = 0;
    bytesWrittenToBody_ = 0;
    isContentChunked_ = false;
    buffer_.clear();
    headers_.clear();
    bodyFile_.close();
    startLine_.path.clear();
    startLine_.uri.clear();
    startLine_.version.clear();
}

RequestParser::RequestState RequestParser::getState() const { return state_; }
size_t RequestParser::getContentLength() const { return contentLength_; }
RequestStartLine const &RequestParser::getStartLine() const { return startLine_; }
http::Headers const &RequestParser::getHeaders() const { return headers_; }

RequestParser &RequestParser::setMaxContentSize(size_t size) {
    LOG_TRACE("RequestParser::setMaxContentSize(): Updating max size to " << size);
    maxContentSize_ = size;
    if (contentLength_ > maxContentSize_)
        return (setError(PAYLOAD_TOO_LARGE), *this);
    return *this;
}

RequestParser::RequestState RequestParser::proceedReadingBody() {
    LOG_TRACE("RequestParser::proceedReadingBody(): Current state=" << state_);
    if (state_ != HEADERS_READY)
        return state_;
    if (!isContentChunked_ && contentLength_ == 0) {
        LOG_DEBUG("RequestParser::proceedReadingBody(): No body to read. State=REQUEST_READY");
        state_ = REQUEST_READY;
    } else {
        LOG_DEBUG(
            "RequestParser::proceedReadingBody(): Proceeding to read body. State=READING_BODY");
        state_ = READING_BODY;
        handleBody();
    }
    return state_;
}

RequestParser::RequestState RequestParser::addIncomingChunk(char const *chunk, size_t size) {
    LOG_TRACE("RequestParser::addIncomingChunk(): Received " << size << " bytes. State=" << state_);
    if (state_ == READING_HEADERS) {
        buffer_.append(chunk, size);
        readHeaders();
    }
    if (state_ == READING_BODY) {
        buffer_.append(chunk, size);
        handleBody();
    }
    return state_;
}

void RequestParser::handleBody() {
    LOG_TRACE("RequestParser::handleBody(): Handling body data");
    if (!bodyFile_.isOpen() && !bodyFile_.open()) {
        LOG_ERROR("RequestParser::handleBody(): Failed to open TempFile. State=ERROR");
        return setError(INTERNAL_SERVER_ERROR);
    }
    if (isContentChunked_) {
        LOG_TRACE("RequestParser::handleBody(): Routing to chunked handler");
        handleChunkedBody();
    } else {
        LOG_TRACE("RequestParser::handleBody(): Routing to content-length handler");
        handleContentLengthBody();
    }
}

void RequestParser::readHeaders() {
    LOG_TRACE("RequestParser::readHeaders(): Checking buffer for headers");
    if (buffer_.length() > maxHeaderSize_) {
        LOG_DEBUG("RequestParser::readHeaders(): Headers size ("
                  << buffer_.length() << ") exceeds maxHeadersSize (" << maxHeaderSize_
                  << "). State=ERROR");
        return setError(BAD_REQUEST);
    }
    size_t contentStartInBuffer_ = buffer_.find("\r\n\r\n", 0, 4);
    if (contentStartInBuffer_ == std::string::npos) {
        LOG_TRACE("RequestParser::readHeaders(): No \\r\\n\\r\\n found. Need more data.");
        return;
    }

    LOG_TRACE("RequestParser::readHeaders(): Found headers, parsing...");
    startLine_ = RequestStartLine::parse(buffer_);
    if (startLine_.method == RequestStartLine::UNKNOWN) {
        LOG_DEBUG("RequestParser::readHeaders(): Invalid request line. State=ERROR");
        return setError(BAD_REQUEST);
    }
    headers_ = http::Headers::parse(buffer_);
    contentLength_ = headers_.getContentLength();

    LOG_DEBUG("RequestParser::readHeaders(): Content-Length=" << contentLength_);
    state_ = HEADERS_READY;
    isContentChunked_ = headers_.isContentChunked();
    if (!isContentChunked_ && contentLength_ == 0) {
        LOG_DEBUG("RequestParser::readHeaders(): Headers parsed. No body. State=REQUEST_READY");
        state_ = REQUEST_READY;
        buffer_.clear();
    } else {
        LOG_DEBUG(
            "RequestParser::readHeaders(): Headers parsed. Body expected. State=HEADERS_READY");
        buffer_.erase(0, contentStartInBuffer_ + 4);
    }
}

void RequestParser::handleChunkedBody() {
    LOG_ERROR(
        "RequestParser::handleChunkedBody(): Chunked parsing is not implemented. State=ERROR");
    setError(NOT_IMPLEMENTED);
}

void RequestParser::handleContentLengthBody() {
    if (bytesWrittenToBody_ == contentLength_) {
        LOG_DEBUG(
            "RequestParser::handleContentLengthBody(): Body already complete. State=REQUEST_READY");
        state_ = REQUEST_READY;
        return;
    }

    size_t bytesLeftToWrite = contentLength_ - bytesWrittenToBody_;
    size_t bytesInBuff = buffer_.length();
    size_t bytesToWrite = std::min(bytesInBuff, bytesLeftToWrite);

    LOG_TRACE("RequestParser::handleContentLengthBody(): "
              << bytesInBuff << " bytes in buffer, " << bytesToWrite << " bytes to write, "
              << bytesWrittenToBody_ << "/" << contentLength_ << " already written");

    if (bytesToWrite + bytesWrittenToBody_ > maxContentSize_) {
        LOG_DEBUG("RequestParser::handleContentLengthBody(): Body size exceeds maxContentSize ("
                  << maxContentSize_ << "). State=ERROR");
        buffer_.clear();
        return setError(PAYLOAD_TOO_LARGE);
    }

    if (bytesToWrite == 0) {
        LOG_TRACE("RequestParser::handleContentLengthBody(): No bytes to write this cycle (need "
                  "more data).");
        return;
    }

    ssize_t written = write(bodyFile_, buffer_.c_str(), bytesToWrite);

    if (written < 0 || (size_t)written != bytesToWrite) {
        LOG_ERROR("RequestParser::handleContentLengthBody(): Write error to TempFile. State=ERROR");
        buffer_.clear();
        return setError(INTERNAL_SERVER_ERROR);
    }

    LOG_TRACE("RequestParser::handleContentLengthBody(): Wrote " << written
                                                                 << " bytes to TempFile");
    buffer_.erase(0, written);
    bytesWrittenToBody_ += written;

    if (bytesWrittenToBody_ == contentLength_) {
        LOG_DEBUG("RequestParser::handleContentLengthBody(): Body complete. State=REQUEST_READY");
        state_ = REQUEST_READY;
    }
}

void RequestParser::setError(HttpStatus status) {
    state_ = ERROR;
    request_.status(status);
}

}; // namespace http
