#include "http/RequestParser.hpp"
#include "common/filesystem.hpp"
#include "http/Headers.hpp"
#include "http/RequestLine.hpp"
#include <algorithm>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace http {

RequestParser::RequestParser(size_t maxContentSize)
    : maxContentSize_(maxContentSize),
      startLine_(reqContext_.requestLine),
      headers_(reqContext_.headers) {
    buffer_.reserve(8192);
    reset();
}

void RequestParser::reset() {
    startLine_.method = RequestLine::UNKNOWN;
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
RequestLine const &RequestParser::getStartLine() const { return startLine_; }
http::Headers const &RequestParser::getHeaders() const { return headers_; }

HttpRequest &RequestParser::getRequestContext() { return reqContext_; }
RequestParser &RequestParser::setMaxContentSize(size_t size) {
    contentLength_ = size;
    return *this;
}

RequestParser &RequestParser::proceedReadingBody() {
    if (state_ != HEADERS_READY)
        return *this;
    if (!isContentChunked_ && contentLength_ == 0)
        state_ = REQUEST_READY;
    else
        state_ = READING_BODY;
    return *this;
}

RequestParser::RequestState RequestParser::addIncomingChunk(char const *chunk, size_t size) {
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
    if (!bodyFile_.isOpen() && !bodyFile_.open()) {
        state_ = ERROR;
        return;
    }
    if (isContentChunked_) {
        handleChunkedBody();
    } else {
        handleContentLengthBody();
    }
}

void RequestParser::readHeaders() {
    size_t contentStartInBuffer_ = buffer_.find("\r\n\r\n", 0, 4);
    if (contentStartInBuffer_ == std::string::npos)
        return;

    startLine_ = RequestLine::parse(buffer_);
    if (startLine_.method == RequestLine::UNKNOWN) {
        state_ = ERROR;
        return;
    }
    headers_ = http::Headers::parse(buffer_);
    contentLength_ = headers_.getConentLength();

    if (contentLength_ > 0 && contentLength_ > maxContentSize_) {
        state_ = ERROR;
        return;
    }
    state_ = HEADERS_READY;
    isContentChunked_ = headers_.isContentChunked();
    if (!isContentChunked_ && contentLength_ == 0) {
        state_ = REQUEST_READY;
        buffer_.clear();
    } else
        buffer_.erase(0, contentStartInBuffer_ + 4);
    return;
}

void RequestParser::handleChunkedBody() {
    std::cerr << "Not implemented" << std::endl;
    state_ = ERROR;
}

void RequestParser::handleContentLengthBody() {
    size_t bytesLeftToWrite = contentLength_ - bytesWrittenToBody_;
    size_t bytesToWrite = std::min(buffer_.length(), bytesLeftToWrite);
    if (bytesToWrite + bytesWrittenToBody_ > maxContentSize_) {
        state_ = ERROR;
        buffer_.clear();
        return;
    }
    ssize_t written = write(bodyFile_, buffer_.c_str(), buffer_.length());
    if (written < 0 || (size_t)written != bytesToWrite) {
        state_ = ERROR;
        buffer_.clear();
        return;
    }
    buffer_.erase(0, written);
    bytesWrittenToBody_ += written;
    if (bytesWrittenToBody_ == contentLength_)
        state_ = REQUEST_READY;
}

}; // namespace http
