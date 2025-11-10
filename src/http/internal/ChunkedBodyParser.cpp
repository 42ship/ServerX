#include "http/internal/ChunkedBodyParser.hpp"

namespace http {

ChunkedBodyParser::ChunkedBodyParser(utils::TempFile &file) : bodyFile_(file) {}

void ChunkedBodyParser::reset() {
    state_ = PARSING_CHUNK_SIZE;
    maxBodySize_ = 0;
    bytesWritten_ = 0;
    chunkBytesLeft_ = 0;
    errorStatus_ = OK;
}

ChunkedBodyParser::State ChunkedBodyParser::state() const { return state_; }
HttpStatus ChunkedBodyParser::errorStatus() const { return errorStatus_; }
void ChunkedBodyParser::setMaxBodySize(size_t size) { maxBodySize_ = size; }
ChunkedBodyParser::State ChunkedBodyParser::setError(HttpStatus status) {
    state_ = ERROR;
    errorStatus_ = status;
    return state_;
}

// TODO: finish this
ChunkedBodyParser::State ChunkedBodyParser::feed(std::string &buffer) {
    (void)buffer;
    return DONE;
}

} // namespace http
