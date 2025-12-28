#pragma once

#include "http/HttpStatus.hpp"
#include <string>

namespace http {

class ChunkedBodyParser {
public:
    enum Status {
        CHUNK_CONTINUE,
        CHUNK_DONE,
        CHUNK_ERROR
    };

    ChunkedBodyParser();

    void reset();

    /**
     * @brief Parse chunked data incrementally.
     * @param input Raw input buffer
     * @param size Size of input
     * @param output Decoded body data (appended)
     * @param bytesConsumed How many bytes were consumed from input
     * @return Status indicating parse state
     */
    Status parse(const char* input, size_t size,
                 std::string& output, size_t& bytesConsumed);

    HttpStatus errorStatus() const;
    void setMaxBodySize(size_t size);

private:
    struct ParseResult {
        bool ok;
        size_t consumed;

        static ParseResult success(size_t n);
        static ParseResult needMore();
        static ParseResult error();
    };

    enum InternalState {
        READING_SIZE,
        READING_DATA,
        READING_TRAILER_CRLF,
        READING_FINAL_CRLF,
        DONE
    };

    typedef ParseResult (ChunkedBodyParser::*StateHandler)(
        const char* input, size_t size, std::string& output);

    ParseResult readSize(const char* input, size_t size, std::string& output);
    ParseResult readData(const char* input, size_t size, std::string& output);
    ParseResult readTrailerCrlf(const char* input, size_t size, std::string& output);
    ParseResult readFinalCrlf(const char* input, size_t size, std::string& output);

    ParseResult setError(HttpStatus status);
    bool parseHex(const std::string& hex, size_t& result) const;

    InternalState state_;
    std::string sizeBuffer_;
    size_t bytesRemainingInChunk_;
    size_t totalBytesWritten_;
    size_t maxBodySize_;
    HttpStatus errorStatus_;
    bool sawCR_;
};

} // namespace http
