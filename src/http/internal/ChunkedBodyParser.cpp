#include "http/internal/ChunkedBodyParser.hpp"
#include <algorithm>
#include <climits>

namespace http {

namespace {

int hexDigitValue(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    return -1;
}

} // anonymous namespace

ChunkedBodyParser::ParseResult ChunkedBodyParser::ParseResult::success(size_t n) {
    ParseResult r;
    r.ok = true;
    r.consumed = n;
    return r;
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::ParseResult::needMore() {
    ParseResult r;
    r.ok = true;
    r.consumed = 0;
    return r;
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::ParseResult::error() {
    ParseResult r;
    r.ok = false;
    r.consumed = 0;
    return r;
}

ChunkedBodyParser::ChunkedBodyParser()
    : state_(READING_SIZE)
    , bytesRemainingInChunk_(0)
    , totalBytesWritten_(0)
    , maxBodySize_(0)
    , errorStatus_(OK)
    , sawCR_(false) {
}

void ChunkedBodyParser::reset() {
    state_ = READING_SIZE;
    sizeBuffer_.clear();
    bytesRemainingInChunk_ = 0;
    totalBytesWritten_ = 0;
    maxBodySize_ = 0;
    errorStatus_ = OK;
    sawCR_ = false;
}

HttpStatus ChunkedBodyParser::errorStatus() const {
    return errorStatus_;
}

void ChunkedBodyParser::setMaxBodySize(size_t size) {
    maxBodySize_ = size;
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::setError(HttpStatus status) {
    state_ = DONE;
    errorStatus_ = status;
    return ParseResult::error();
}

size_t ChunkedBodyParser::findCRLF(const char* input, size_t size) const {
    for (size_t i = 0; i + 1 < size; ++i) {
        if (input[i] == '\r' && input[i + 1] == '\n')
            return i;
    }
    return NPOS;
}

bool ChunkedBodyParser::parseHex(const std::string& hex, size_t& result) const {
    if (hex.empty())
        return false;

    result = 0;
    for (size_t i = 0; i < hex.size(); ++i) {
        int digit = hexDigitValue(hex[i]);
        if (digit < 0)
            return false;
        if (result > (static_cast<size_t>(-1) - digit) / 16)
            return false;
        result = result * 16 + digit;
    }
    return true;
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::readSize(
    const char* input, size_t size, std::string& output) {
    (void)output;

    size_t prevBufferLen = sizeBuffer_.size();
    sizeBuffer_.append(input, size);

    size_t crlfPos = findCRLF(sizeBuffer_.c_str(), sizeBuffer_.size());
    if (crlfPos == NPOS)
        return ParseResult::success(size);

    std::string hex = sizeBuffer_.substr(0, crlfPos);
    size_t lineEndInBuffer = crlfPos + 2;
    size_t consumedFromInput = 0;
    if (lineEndInBuffer > prevBufferLen)
        consumedFromInput = lineEndInBuffer - prevBufferLen;

    sizeBuffer_.clear();

    size_t chunkSize = 0;
    if (!parseHex(hex, chunkSize))
        return setError(BAD_REQUEST);

    if (chunkSize == 0) {
        state_ = READING_FINAL_CRLF;
        return ParseResult::success(consumedFromInput);
    }

    if (maxBodySize_ > 0 && totalBytesWritten_ + chunkSize > maxBodySize_)
        return setError(PAYLOAD_TOO_LARGE);

    bytesRemainingInChunk_ = chunkSize;
    state_ = READING_DATA;
    return ParseResult::success(consumedFromInput);
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::readData(
    const char* input, size_t size, std::string& output) {

    size_t toRead = std::min(size, bytesRemainingInChunk_);

    if (maxBodySize_ > 0 && totalBytesWritten_ + toRead > maxBodySize_)
        return setError(PAYLOAD_TOO_LARGE);

    output.append(input, toRead);
    totalBytesWritten_ += toRead;
    bytesRemainingInChunk_ -= toRead;

    if (bytesRemainingInChunk_ == 0)
        state_ = READING_TRAILER_CRLF;

    return ParseResult::success(toRead);
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::readTrailerCrlf(
    const char* input, size_t size, std::string& output) {
    (void)output;

    if (sawCR_) {
        if (input[0] != '\n')
            return setError(BAD_REQUEST);
        sawCR_ = false;
        state_ = READING_SIZE;
        return ParseResult::success(1);
    }

    if (size < 2) {
        if (input[0] != '\r')
            return setError(BAD_REQUEST);
        sawCR_ = true;
        return ParseResult::success(1);
    }

    if (input[0] != '\r' || input[1] != '\n')
        return setError(BAD_REQUEST);

    state_ = READING_SIZE;
    return ParseResult::success(2);
}

ChunkedBodyParser::ParseResult ChunkedBodyParser::readFinalCrlf(
    const char* input, size_t size, std::string& output) {
    (void)output;

    if (sawCR_) {
        if (input[0] != '\n')
            return setError(BAD_REQUEST);
        sawCR_ = false;
        state_ = DONE;
        return ParseResult::success(1);
    }

    if (size < 2) {
        if (input[0] != '\r')
            return setError(BAD_REQUEST);
        sawCR_ = true;
        return ParseResult::success(1);
    }

    if (input[0] != '\r' || input[1] != '\n')
        return setError(BAD_REQUEST);

    state_ = DONE;
    return ParseResult::success(2);
}

ChunkedBodyParser::Status ChunkedBodyParser::parse(
    const char* input, size_t size,
    std::string& output, size_t& bytesConsumed) {

    static const StateHandler handlers[] = {
        &ChunkedBodyParser::readSize,
        &ChunkedBodyParser::readData,
        &ChunkedBodyParser::readTrailerCrlf,
        &ChunkedBodyParser::readFinalCrlf,
        NULL
    };

    bytesConsumed = 0;
    size_t pos = 0;

    while (pos < size && state_ != DONE) {
        StateHandler handler = handlers[state_];
        if (handler == NULL)
            break;

        ParseResult result = (this->*handler)(input + pos, size - pos, output);

        if (!result.ok)
            return CHUNK_ERROR;

        if (result.consumed == 0)
            break;

        pos += result.consumed;
    }

    bytesConsumed = pos;

    if (state_ == DONE)
        return CHUNK_DONE;
    return CHUNK_CONTINUE;
}

} // namespace http
