Design: Clean Patterns

### Summary of changes
  1. inc/http/internal/ChunkedBodyParser.hpp
    - New parse() interface with (input, size, output, bytesConsumed) signature
    - Internal ParseResult struct for clean error handling
    - Dispatch table pattern via StateHandler typedef
    - Added sawCR_ flag for handling partial CRLF
  2. src/http/internal/ChunkedBodyParser.cpp
    - Dispatch table (no switch-case): handlers[state_]
    - Guard clauses with early returns (no nested if-else)
    - Safe hex parsing with overflow protection
    - Proper incremental parsing for byte-by-byte input
  3. src/http/RequestParser.cpp
    - Updated constructor (no longer passes bodyFile_ to ChunkedBodyParser)
    - New writeToBodyFile() helper
    - Implemented handleChunkedBody() using new ChunkedBodyParser interface
  4. tests/http/ChunkedBodyParser.cpp (new file)
    - Tests for single/multiple chunks, incremental parsing
    - Byte-by-byte parsing test
    - Error handling tests
    - Size limit tests
    - Reset functionality test



### Implementation Order

 1. Define ParseResult struct in header
 2. Implement hexDigitValue() helper (pure function)
 3. Implement parseHex() using guard clauses
 4. Implement each state handler (readSize, readData, readTrailerCrlf, readFinalCrlf)
 5. Implement parse() with dispatch table
 6. Add writeToBodyFile() helper to RequestParser
 7. Update handleChunkedBody() with guard clauses
 8. Write unit tests
 9. Run make test

 1. Result Pattern (Internal Only)

 // Internal result type - doesn't affect rest of codebase
 struct ParseResult {
     bool ok;
     size_t consumed;  // bytes consumed from input

     static ParseResult success(size_t n) { return (ParseResult){true, n}; }
     static ParseResult needMore() { return (ParseResult){true, 0}; }
     static ParseResult error() { return (ParseResult){false, 0}; }
 };

 2. Dispatch Table (No Switch-Case)

 // Each state has its own handler method
 typedef ParseResult (ChunkedBodyParser::*StateHandler)(
     const char* input, size_t size, std::string& output);

 // Flat dispatch - no switch needed
 Status ChunkedBodyParser::parse(...) {
     static const StateHandler handlers[] = {
         &ChunkedBodyParser::readSize,
         &ChunkedBodyParser::readData,
         &ChunkedBodyParser::readTrailer
     };

     ParseResult result = (this->*handlers[state_])(input, size, output);

     if (!result.ok)
         return CHUNK_ERROR;
     bytesConsumed = result.consumed;
     return (state_ == DONE) ? CHUNK_DONE : CHUNK_CONTINUE;
 }

 3. Guard Clauses (No Nested If-Else)

 ParseResult ChunkedBodyParser::readSize(const char* input, size_t size, std::string& out) {
     size_t crlfPos = findCRLF(input, size);

     // Guard: incomplete line
     if (crlfPos == NPOS)
         return accumulateSizeBuffer(input, size);

     // Guard: parse hex
     size_t chunkSize = 0;
     if (!parseHex(sizeBuffer_ + std::string(input, crlfPos), chunkSize))
         return setError(BAD_REQUEST);

     // Guard: final chunk
     if (chunkSize == 0) {
         state_ = READING_FINAL_CRLF;
         return ParseResult::success(crlfPos + 2);
     }

     // Success path
     bytesRemainingInChunk_ = chunkSize;
     sizeBuffer_.clear();
     state_ = READING_DATA;
     return ParseResult::success(crlfPos + 2);
 }

 4. Single-Pass Data Reading (No Nested Loops)

 ParseResult ChunkedBodyParser::readData(const char* input, size_t size, std::string& out) {
     size_t toRead = std::min(size, bytesRemainingInChunk_);

     // Guard: body size limit
     if (totalBytesWritten_ + toRead > maxBodySize_)
         return setError(PAYLOAD_TOO_LARGE);

     out.append(input, toRead);
     totalBytesWritten_ += toRead;
     bytesRemainingInChunk_ -= toRead;

     // Transition when chunk complete
     if (bytesRemainingInChunk_ == 0)
         state_ = READING_TRAILER_CRLF;

     return ParseResult::success(toRead);
 }

 ---
 ChunkedBodyParser.hpp (Clean Version)

 #pragma once

 #include "http/HttpStatus.hpp"
 #include <string>

 namespace http {

 class ChunkedBodyParser {
 public:
     enum Status { CHUNK_CONTINUE, CHUNK_DONE, CHUNK_ERROR };

     ChunkedBodyParser();
     void reset();

     Status parse(const char* input, size_t size,
                  std::string& output, size_t& bytesConsumed);

     HttpStatus errorStatus() const;
     void setMaxBodySize(size_t size);

 private:
     // Internal result type for clean error handling
     struct ParseResult {
         bool ok;
         size_t consumed;
         static ParseResult success(size_t n);
         static ParseResult needMore();
         static ParseResult error();
     };

     enum InternalState { READING_SIZE, READING_DATA, READING_TRAILER_CRLF, READING_FINAL_CRLF, DONE };

     // One method per state - no switch needed
     ParseResult readSize(const char* input, size_t size, std::string& output);
     ParseResult readData(const char* input, size_t size, std::string& output);
     ParseResult readTrailerCrlf(const char* input, size_t size, std::string& output);
     ParseResult readFinalCrlf(const char* input, size_t size, std::string& output);

     // Helpers
     ParseResult setError(HttpStatus status);
     bool parseHex(const std::string& hex, size_t& result) const;

     InternalState state_;
     std::string sizeBuffer_;
     size_t bytesRemainingInChunk_;
     size_t totalBytesWritten_;
     size_t maxBodySize_;
     HttpStatus errorStatus_;
 };

 } // namespace http

 ---
 RequestParser::handleChunkedBody() (Clean Version)

 void RequestParser::handleChunkedBody() {
     std::string decodedData;
     size_t bytesConsumed = 0;

     ChunkedBodyParser::Status status =
         chunkParser_.parse(buffer_.c_str(), buffer_.size(), decodedData, bytesConsumed);

     buffer_.erase(0, bytesConsumed);

     // Guard: write error
     if (!decodedData.empty() && !writeToBodyFile(decodedData))
         return;

     // Guard: parser error
     if (status == ChunkedBodyParser::CHUNK_ERROR) {
         setError(chunkParser_.errorStatus());
         return;
     }

     // Success: check if done
     if (status == ChunkedBodyParser::CHUNK_DONE)
         setRequestReady();
 }

 bool RequestParser::writeToBodyFile(const std::string& data) {
     ssize_t written = write(bodyFile_, data.c_str(), data.size());
     if (written < 0) {
         setError(INTERNAL_SERVER_ERROR);
         return false;
     }
     bytesWrittenToBody_ += written;
     return true;
 }

 ---
 Hex Parsing (Clean, No Nested Ifs)

 bool ChunkedBodyParser::parseHex(const std::string& hex, size_t& result) const {
     if (hex.empty())
         return false;

     result = 0;
     for (size_t i = 0; i < hex.size(); ++i) {
         int digit = hexDigitValue(hex[i]);
         if (digit < 0)
             return false;
         if (result > (SIZE_MAX - digit) / 16)
             return false;  // Overflow
         result = result * 16 + digit;
     }
     return true;
 }

 // Pure function - no state, single responsibility
 int hexDigitValue(char c) {
     if (c >= '0' && c <= '9') return c - '0';
     if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
     if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
     return -1;
 }

 ---
 Pattern Summary

 | Bad Pattern              | Clean Alternative                |
 |--------------------------|----------------------------------|
 | switch (state_)          | Dispatch table: handlers[state_] |
 | Nested if-else           | Guard clauses with early return  |
 | Nested for loops         | Single-pass processing           |
 | Error codes scattered    | ParseResult type                 |
 | State + error in returns | Separate concerns                |


