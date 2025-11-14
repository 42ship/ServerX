#pragma once

#include "common/filesystem.hpp"
#include "http/HttpStatus.hpp"
#include <string>

namespace http {

class ChunkedBodyParser {
public:
    enum State {
        PARSING_CHUNK_SIZE, // Waiting for size (e.g., "A0\r\n")
        PARSING_CHUNK_DATA, // Reading the N bytes of data
        PARSING_TRAILER,    // Reading the final '\r\n' after the data
        DONE,               // Saw "0\r\n\r\n"
        ERROR
    };

    ChunkedBodyParser(utils::TempFile &file);

    void reset();

    /**
     * @brief Consumes data from the buffer, parses, and writes to the file.
     * @param buffer The RequestParser's main data buffer.
     * @return The new internal state.
     */
    State feed(std::string &buffer);

    State state() const;
    HttpStatus errorStatus() const;
    void setMaxBodySize(size_t size);

private:
    State setError(HttpStatus status);

    State state_;
    utils::TempFile &bodyFile_;
    size_t maxBodySize_;
    size_t bytesWritten_;
    size_t chunkBytesLeft_;
    HttpStatus errorStatus_;
};

} // namespace http
