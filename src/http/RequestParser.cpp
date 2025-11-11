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

RequestParser::RequestParser(Request &req, size_t maxHeaderSize)
    : maxHeaderSize_(maxHeaderSize), request_(req), chunkParser_(bodyFile_) {
    LOG_TRACE("RequestParser::RequestParser(): Initializing...");
    reset();
}

void RequestParser::reset() {
    LOG_TRACE("RequestParser::reset(): Resetting parser state");
    state_ = READING_HEADERS;
    errorStatus_ = OK;
    contentLength_ = 0;
    bytesWrittenToBody_ = 0;
    maxContentSize_ = 0;
    isContentChunked_ = false;
    buffer_.clear();
    bodyFile_.close();
    request_.clear();
}

RequestParser::State RequestParser::state() const { return state_; }
HttpStatus RequestParser::errorStatus() const { return errorStatus_; }

RequestParser::State RequestParser::setError(HttpStatus status) {
    reset();
    errorStatus_ = status;
    state_ = ERROR;
    return state_;
}

RequestParser::State RequestParser::feed(char const *chunk, size_t size) {
    if (state_ == READING_HEADERS) {
        buffer_.append(chunk, size);
        parseHeaders();
    }
    if (state_ == READING_BODY) {
        buffer_.append(chunk, size);
        parseBody();
    }
    return state_;
}

RequestParser::State RequestParser::proceedReadingBody() {
    if (state_ != HEADERS_READY)
        return state_;
    state_ = READING_BODY;
    if (isContentChunked_) {
        chunkParser_.setMaxBodySize(request_.getMaxAllowedContentSize());
        return parseBody();
    }
    if (contentLength_ > 0) {
        if (contentLength_ > request_.getMaxAllowedContentSize())
            return setError(PAYLOAD_TOO_LARGE);
        return parseBody();
    }
    state_ = REQUEST_READY;
    return state_;
}

RequestParser::State RequestParser::parseHeaders() {
    if (buffer_.length() > maxHeaderSize_) {
        return setError(BAD_REQUEST);
    }
    size_t contentStartInBuffer_ = buffer_.find("\r\n\r\n", 0, 4);
    if (contentStartInBuffer_ == std::string::npos) {
        return state_;
    }
    state_ = HEADERS_READY;
    request_.requestLine_ = RequestStartLine::parse(buffer_);
    if (request_.method() == RequestStartLine::UNKNOWN) {
        return setError(BAD_REQUEST);
    }
    // Only parse the header section, not the body
    std::string headerSection = buffer_.substr(0, contentStartInBuffer_);
    request_.headers_ = http::Headers::parse(headerSection);
    contentLength_ = request_.headers_.getContentLength();
    isContentChunked_ = request_.headers_.isContentChunked();
    bool isBodyExpected = contentLength_ > 0 || isContentChunked_;
    if (isBodyExpected) {
        buffer_.erase(0, contentStartInBuffer_ + 4);
    } else {
        buffer_.clear();
        state_ = REQUEST_READY;
    }
    return state_;
}

RequestParser::State RequestParser::parseBody() {
    if (!bodyFile_.isOpen() && !bodyFile_.open()) {
        LOG_ERROR("RequestParser::handleBody(): Failed to open TempFile. State=ERROR");
        return setError(INTERNAL_SERVER_ERROR);
    }
    if (isContentChunked_) {
        handleChunkedBody();
    } else {
        handleContentLengthBody();
    }
    if (state_ == REQUEST_READY && bodyFile_.isOpen()) {
        if (lseek(bodyFile_, 0, SEEK_SET) != (off_t)-1)
            request_.body(bodyFile_);
    }
    return state_;
}

void RequestParser::handleContentLengthBody() {
    if (bytesWrittenToBody_ == contentLength_) {
        state_ = REQUEST_READY;
        return;
    }
    size_t bytesLeftToWrite = contentLength_ - bytesWrittenToBody_;
    size_t bytesToWrite = std::min(buffer_.length(), bytesLeftToWrite);
    if (!bytesToWrite)
        return;
    ssize_t written = write(bodyFile_, buffer_.c_str(), bytesToWrite);

    if (written < 0) {
        LOG_ERROR("RequestParser::handleContentLengthBody(): Write error to TempFile.");
        return (void)setError(INTERNAL_SERVER_ERROR);
    }

    buffer_.erase(0, written);
    bytesWrittenToBody_ += written;

    if (bytesWrittenToBody_ == contentLength_) {
        state_ = REQUEST_READY;
    }
}

void RequestParser::handleChunkedBody() {
    LOG_ERROR(
        "RequestParser::handleChunkedBody(): Chunked parsing is not implemented. State=ERROR");
    ChunkedBodyParser::State state = chunkParser_.feed(buffer_);
    (void)state;
    setError(NOT_IMPLEMENTED);
}

std::ostream &operator<<(std::ostream &o, RequestParser::State st) {
    if (st == RequestParser::REQUEST_READY)
        o << "REQUEST READY";
    else if (st == RequestParser::HEADERS_READY)
        o << "HEADERS READY";
    else if (st == RequestParser::READING_BODY)
        o << "READING BODY";
    else if (st == RequestParser::READING_HEADERS)
        o << "READING HEADERS";
    else
        o << "ERROR";
    return o;
}

}; // namespace http
