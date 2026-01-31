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
    : maxHeaderSize_(maxHeaderSize), request_(req), chunkParser_() {
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
    // buffer_.clear(); // Preserve buffer for pipelined requests
    request_.clear();
}

RequestParser::State RequestParser::state() const { return state_; }
HttpStatus RequestParser::errorStatus() const { return errorStatus_; }

RequestParser::State RequestParser::setError(HttpStatus status) {
    LOG_SDEBUG("Parser error status set to: " << status);
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
    setRequestReady();
    return state_;
}

RequestParser::State RequestParser::parseHeaders() {
    // RFC 7230: skip leading CRLFs
    size_t start = buffer_.find_first_not_of("\r\n");
    if (start != std::string::npos && start > 0) {
        buffer_.erase(0, start);
    } else if (start == std::string::npos && !buffer_.empty()) {
        // Only CRLFs in buffer, keep them but maybe keep buffer small
        if (buffer_.length() > 1024)
            buffer_.clear();
        return state_;
    }

    if (buffer_.length() > maxHeaderSize_) {
        LOG_SDEBUG("Header size limit exceeded: " << buffer_.length() << " > " << maxHeaderSize_);
        return setError(BAD_REQUEST);
    }

    size_t offset = 0;
    size_t headerEnd = http::Headers::findHeaderEnd(buffer_, offset);

    if (headerEnd == std::string::npos) {
        return state_;
    }

    state_ = HEADERS_READY;
    request_.requestLine_ = RequestStartLine::parse(buffer_);
    if (request_.method() == RequestStartLine::UNKNOWN) {
        LOG_SDEBUG("Unknown method or failed to parse request line: "
                   << buffer_.substr(0, buffer_.find('\n')));
        return setError(BAD_REQUEST);
    }

    // Only parse the header section, not the body
    size_t headerSectionStart = buffer_.find('\n');
    if (headerSectionStart == std::string::npos) {
        LOG_SDEBUG("Failed to find end of request line even though delimiter was found");
        return setError(BAD_REQUEST);
    }
    headerSectionStart += 1;

    std::string headerSection =
        buffer_.substr(headerSectionStart, headerEnd - headerSectionStart + (offset - 2));
    if (!http::Headers::parse(headerSection, request_.headers_)) {
        LOG_SDEBUG("Failed to parse header section: " << headerSection);
        return setError(BAD_REQUEST);
    }
    contentLength_ = request_.headers_.getContentLength();
    isContentChunked_ = request_.headers_.isContentChunked();
    bool isBodyExpected = contentLength_ > 0 || isContentChunked_;
    if (isBodyExpected) {
        buffer_.erase(0, headerEnd + offset);
    } else {
        buffer_.erase(0, headerEnd + offset);
        setRequestReady();
    }
    return state_;
}

RequestParser::State RequestParser::parseBody() {
    if (!request_.body_) {
        request_.body_ = new utils::TempFile();
    }
    if (!request_.body_->isOpen() && !request_.body_->open()) {
        LOG_SERROR("Failed to open TempFile. State=ERROR");
        return setError(INTERNAL_SERVER_ERROR);
    }
    if (isContentChunked_) {
        handleChunkedBody();
    } else {
        handleContentLengthBody();
    }
    return state_;
}

void RequestParser::handleContentLengthBody() {
    if (bytesWrittenToBody_ == contentLength_) {
        setRequestReady();
        return;
    }
    size_t bytesLeftToWrite = contentLength_ - bytesWrittenToBody_;
    size_t bytesToWrite = std::min(buffer_.length(), bytesLeftToWrite);
    if (!bytesToWrite)
        return;
    ssize_t written = write(request_.body_->fd(), buffer_.c_str(), bytesToWrite);

    if (written < 0) {
        LOG_SERROR("Write error to TempFile.");
        return (void)setError(INTERNAL_SERVER_ERROR);
    }

    buffer_.erase(0, written);
    bytesWrittenToBody_ += written;

    if (bytesWrittenToBody_ == contentLength_) {
        setRequestReady();
    }
}

bool RequestParser::writeToBodyFile(const std::string &data) {
    if (!request_.body_)
        return false;
    ssize_t written = write(request_.body_->fd(), data.c_str(), data.size());
    if (written < 0) {
        LOG_ERROR("RequestParser::writeToBodyFile(): Write error to TempFile.");
        setError(INTERNAL_SERVER_ERROR);
        return false;
    }
    bytesWrittenToBody_ += written;
    return true;
}

void RequestParser::handleChunkedBody() {
    std::string decodedData;
    size_t bytesConsumed = 0;

    ChunkedBodyParser::Status status =
        chunkParser_.parse(buffer_.c_str(), buffer_.size(), decodedData, bytesConsumed);

    buffer_.erase(0, bytesConsumed);

    if (!decodedData.empty()) {
        LOG_STRACE("decoded " << decodedData.size() << " bytes: \"" << decodedData << "\"");
        if (!writeToBodyFile(decodedData))
            return;
    }

    if (status == ChunkedBodyParser::CHUNK_ERROR) {
        LOG_STRACE("parse error, status=" << chunkParser_.errorStatus());
        setError(chunkParser_.errorStatus());
        return;
    }

    if (status == ChunkedBodyParser::CHUNK_DONE) {
        LOG_STRACE("complete, total bytes=" << bytesWrittenToBody_);
        setRequestReady();
    }
}

void RequestParser::setRequestReady() {
    state_ = REQUEST_READY;
    if (request_.body_ && request_.body_->isOpen()) {
        if (lseek(request_.body_->fd(), 0, SEEK_SET) == (off_t)-1) {
            LOG_ERROR("RequestParser::setRequestReady(): lseek failed on TempFile. State=ERROR");
            setError(INTERNAL_SERVER_ERROR);
        }
    }
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
