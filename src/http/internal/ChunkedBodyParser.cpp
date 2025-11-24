#include "http/internal/ChunkedBodyParser.hpp"

namespace http {

ChunkedBodyParser::ChunkedBodyParser(utils::TempFile &file)
    : state_(PARSING_CHUNK_SIZE), bodyFile_(file), maxBodySize_(0), bytesWritten_(0),
      chunkBytesLeft_(0), errorStatus_(OK) {}

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

// TODO: finish this implementation (will modify state_ and bodyFile_ when implemented)
ChunkedBodyParser::State ChunkedBodyParser::feed(std::string &buffer) {
    (void)buffer;
    (void)bodyFile_;
    return DONE;
}

} // namespace http
