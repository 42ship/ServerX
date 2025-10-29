#include "Headers.hpp"
#include "RequestLine.hpp"
#include "TempFile.hpp"
#include <string>
#include <sys/types.h>

class RequestParser {
public:
    enum RequestState { READING_HEADERS, READING_BODY, REQUEST_READY, ERROR };

    RequestParser() {
        buffer_.reserve(8192);
        reset();
    }

    void reset() {
        buffer_.clear();
        state_ = READING_HEADERS;
        contentLength_ = -1;
        isBodyExpected_ = false;
        isContentChunked_ = false;
        maxContentSize_ = 0;
        contentLength_ = 0;
        contentStartInBuffer_ = 0;
        headers_.clear();
        bodyFile_.close();
        startLine_.method = RequestLine::UNKNOWN;
        startLine_.path.clear();
        startLine_.uri.clear();
        startLine_.version.clear();
    }

    RequestState addIncomingChunk(char const *chunk, size_t size) {
        if (state_ == READING_HEADERS) {
            readHeaders(chunk, size);
        }
        if (state_ == READING_BODY) {
            handleBody(chunk, size);
        }

        return state_;
    }

private:
    std::string buffer_;

    // Properties
    size_t maxContentSize_;

    // State of the request
    RequestState state_;
    bool isContentChunked_;
    bool isBodyExpected_;

    size_t contentLength_;
    size_t contentStartInBuffer_;

    // Final objects
    RequestLine startLine_;
    http::Headers headers_;
    TempFile bodyFile_;
    // Final objects

    void readHeaders(char const *chunk, size_t size) {
        buffer_.append(chunk);

        contentStartInBuffer_ = buffer_.find("\r\n\r\n", 0, 4);
        if (contentStartInBuffer_ == std::string::npos)
            return;

        contentStartInBuffer_ += 4;
        state_ = READING_BODY;
        headers_ = http::Headers::parse(buffer_);
        startLine_ = RequestLine::parse(buffer_);
        contentLength_ = headers_.getConentLength();
        isContentChunked_ = headers_.isContentChunked();
        // In future integrate with ServerConfig to check if it's expected and allowed
        isBodyExpected_ = true;
    }

    void handleBody(char const *chunk, size_t size) {
        if (!isBodyExpected_) {
            // TODO: make some sort of error that Router may use to give a correct handler
            return;
        }
        if (!buffer_.empty()) {
            chunk = buffer_.data() + contentStartInBuffer_;
            size = buffer_.size() - contentStartInBuffer_;
            buffer_.clear();
        }
        (void)chunk;
        (void)size;
    }
};

/*
Q: What is my problem?
A: Currently in order to properly handle clinet's body we need to know this:
    1) Whether the body is allowed for  the current request
    2) The size of incoming body
    3) If the body is chuncked it needs to be handled properly

 * */
