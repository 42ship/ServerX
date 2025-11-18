#include "http/RequestParser.hpp"
#include "doctest.h"
#include "utils/Logger.hpp"

/*
clang-format off

**A. `RequestParser` (The "Dumb" Parser's Job)**

The parser's job is to read bytes, follow HTTP syntax rules, and report syntax errors.

1.  **Header Size Limit:**
    * **Situation:** Client sends more than `kMaxHeaderSize` (e.g., 8KB) *before* sending a `\r\n\r\n`.
    * **Action:** `RequestParser::readHeaders` sets `state_ = ERROR` and `request_.setStatus(PAYLOAD_TOO_LARGE)`.
    * **Result:** `Reactor` sends a `413 Payload Too Large`.

2.  **Pipelined Data :**
    * **Situation:** Client sends more data than its advertised `Content-Length`.
    * **Action:** `RequestParser::handleContentLengthBody` stops reading at `contentLength_`. The extra data is left in the `buffer_`.
    * **Result:** `state_` becomes `REQUEST_READY`. The `Reactor` sends the response, and on `reset()`, the parser re-processes the leftover buffer as a new request.

3.  **Malformed Request Line:**
    * **Situation:** Client sends `GET /path` (missing version) or `INVALID_METHOD /path HTTP/1.1`.
    * **Action:** `RequestStartLine::parse` fails, `startLine_.method` remains `UNKNOWN`.
    * **Result:** `RequestParser::readHeaders` sets `state_ = ERROR` and `request_.setStatus(BAD_REQUEST)`. `Reactor` sends `400 Bad Request`.

4.  **Malformed Chunked Body:**
    * **Situation:** Client sends an invalid chunk size (e.g., "abc\r\n") or a chunk's data doesn't match its advertised size.
    * **Action:** `RequestParser::handleChunkedBody` (when implemented) fails.
    * **Result:** `state_ = ERROR` and `request_.setStatus(BAD_REQUEST)`. `Reactor` sends `400 Bad Request`.

#### **B. `Reactor` (The "Smart" Controller's Job)**

The `Reactor` coordinates the parser, router, and configuration to enforce *semantic* rules.

5.  **Route-Specific Size Limit:**
    * **Situation:** `Content-Length: 5000`, but the specific location's `client_max_body_size` is `1000`.
    * **Action:** In `handleRead` (at `HEADERS_READY` state), the `Reactor` gets `specificLimit = 1000`, sees `5000 > 1000`.
    * **Result:** `Reactor` calls `request_.setStatus(PAYLOAD_TOO_LARGE)` and `reqParser_.skipBodyAndSetReady()`, then calls `generateResponse()` to send the `413` error.

6.  **Global Size Limit:**
    * **Situation:** `Content-Length: 5000`, and the location has *no* `client_max_body_size` set.
    * **Action:** In `handleRead`, `specificLimit` falls back to the *global* limit (e.g., `1000`).
    * **Result:** Same as #5. The request is rejected with `413`.

7.  **Chunked Size Limit:**
    * **Situation:** A chunked request is received. The `Reactor` sets `reqParser_.setMaxContentSize(specificLimit)`.
    * **Action:** `RequestParser::handleChunkedBody` (when implemented) must sum the bytes as they arrive.
    * **Result:** If `total_bytes_written` exceeds `maxContentSize_`, `handleChunkedBody` sets `state_ = ERROR` and `request_.setStatus(PAYLOAD_TOO_LARGE)`.

8.  **Body on `GET`/`HEAD`:**
    * **Situation:** `GET /path HTTP/1.1` with `Content-Length: 1000`.
    * **Action:** In `handleRead`, the `Router` finds the route. The `LocationBlock` logic should specify `client_max_body_size: 0` for `GET` methods.
    * **Result:** `specificLimit` becomes `0`. The `Reactor` sees `1000 > 0` and rejects with `413 Payload Too Large`. (This is a robust way to "ignore" it).

9.  **Missing Length:**
    * **Situation:** `POST /path HTTP/1.1` arrives with *no* `Content-Length` and *no* `Transfer-Encoding: chunked`.
    * **Action:** In `handleRead`, the `Reactor` sees `state_ == HEADERS_READY`. It checks `request_.method()`, `parser.getContentLength()`, and `parser.isChunked()`.
    * **Result:** The `Reactor` calls `request_.setStatus(LENGTH_REQUIRED)` and `generateResponse()` to send the `411` error.

#### **C. `Headers` (The "Special Case" Rules)**

These are complex HTTP rules that the `Headers` class itself should handle.

10. **Conflicting Body Headers:**
    * **Situation:** Request contains *both* `Content-Length` and `Transfer-Encoding: chunked`.
    * **Action:** The RFC states `Transfer-Encoding` wins. `Headers::parse` should ignore `Content-Length`.
    * **Result:** `headers_.getContentLength()` returns `0`, and `headers_.isContentChunked()` returns `true`. The parser correctly enters chunked mode.

11. **Multiple `Content-Length` Headers:**
    * **Situation:** Request contains `Content-Length: 100` and `Content-Length: 200`.
    * **Action:** This is ambiguous and invalid. `Headers::parse` should detect this.
    * **Result:** `Headers::parse` should throw an exception (or return a "bad" state) that the `RequestParser` catches, setting `state_ = ERROR` and `request_.setStatus(BAD_REQUEST)`.
clang-format on
*/

TEST_CASE("RequestParser Testing handling limits") {
    http::Request req;

    SUBCASE("Header Size Limit") {
        http::RequestParser rp(req, 10);
        rp.feed("Hello World!!", 13);
        REQUIRE(rp.state() == http::RequestParser::ERROR);
        CHECK(rp.errorStatus() == http::BAD_REQUEST);
    }

    SUBCASE("Malformed Request Line") {
        http::RequestParser rp(req, 8192);
        REQUIRE(rp.feed("BLAHDF /hello \r\n\r\n", 18) == http::RequestParser::ERROR);
        CHECK(rp.errorStatus() == http::BAD_REQUEST);
    }
}

// TEST_CASE("RequestParser basic functions") { http::Request req; }
