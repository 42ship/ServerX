#include "doctest.h"
#include "http/HttpRequest.hpp"
#include "http/utils.hpp"
#include <sstream>

using namespace std;
using namespace http;
using namespace utils;

bool testStartLine(const std::string &line, HttpRequest &req) {
    req = HttpRequest();
    istringstream stream(line);
    return details::parseStartLine(req, stream);
}

bool testHeaders(const std::string &headerBlock, HttpRequest &req) {
    req = HttpRequest(); // Reset for a clean state
    istringstream stream(headerBlock);
    return details::parseHeaders(req.headers, stream);
}

TEST_CASE("HTTP Request Start Line Parsing") {
    HttpRequest r;

    SUBCASE("Valid Methods") {
        REQUIRE(testStartLine("GET /index.html HTTP/1.1\r\n", r));
        CHECK(r.method == GET);
        CHECK(r.uri == "/index.html");
        CHECK(r.version == "HTTP/1.1");

        REQUIRE(testStartLine("POST /api/users HTTP/1.1\r\n", r));
        CHECK(r.method == POST);
        CHECK(r.uri == "/api/users");

        REQUIRE(testStartLine("DELETE /items/123 HTTP/1.0\r\n", r));
        CHECK(r.method == DELETE);
        CHECK(r.uri == "/items/123");
        CHECK(r.version == "HTTP/1.0");

        REQUIRE(testStartLine("PUT /files/new.txt HTTP/1.1\r\n", r));
        CHECK(r.method == PUT);

        REQUIRE(testStartLine("OPTIONS * HTTP/1.1\r\n", r));
        CHECK(r.method == UNKNOWN);
    }

    SUBCASE("Whitespace Tolerance") {
        REQUIRE(testStartLine("  GET   /path/to/resource   HTTP/1.1  \r\n", r));
        CHECK(r.method == GET);
        CHECK(r.uri == "/path/to/resource");
        CHECK(r.version == "HTTP/1.1");

        REQUIRE(testStartLine("\tPOST\t/\tHTTP/1.1\t\r\n", r));
        CHECK(r.method == POST);
        CHECK(r.uri == "/");
    }

    SUBCASE("URI Variations") {
        REQUIRE(testStartLine("GET / HTTP/1.1\r\n", r));
        CHECK(r.uri == "/");

        REQUIRE(testStartLine("GET /a/b/c/d/e/f/g HTTP/1.1\r\n", r));
        CHECK(r.uri == "/a/b/c/d/e/f/g");

        REQUIRE(testStartLine("GET /with?query=value&another=param HTTP/1.1\r\n", r));
        CHECK(r.uri == "/with?query=value&another=param");
    }

    SUBCASE("Malformed or Invalid Start Lines") {
        CHECK_FALSE(testStartLine("", r)); // Empty string
        // CHECK_FALSE(testStartLine("GET / HTTP/1.1", r));           // Missing CRLF
        // CHECK_FALSE(testStartLine("GET / HTTP/1.1\n", r));         // LF only, not CRLF
        CHECK_FALSE(testStartLine("GET /\r\n", r));        // Missing version
        CHECK_FALSE(testStartLine("GET HTTP/1.1\r\n", r)); // Missing URI
        CHECK_FALSE(testStartLine("/ HTTP/1.1\r\n", r));   // Missing method
        // CHECK_FALSE(testStartLine("GET / EXTRA HTTP/1.1\r\n", r)); // Too many parts
        // CHECK_FALSE(testStartLine("get / http/1.1\r\n", r)); // Invalid case for method/version
        REQUIRE(testStartLine("GET / HTTP/1.1\r\nEXTRA_DATA", r)); // Extra data after CRLF
    }
}

TEST_CASE("HTTP Request Header Parsing" * doctest::skip(true)) {
    HttpRequest r;

    SUBCASE("Valid Headers") {
        const char *headers = "Host: www.example.com\r\n"
                              "User-Agent: TestClient/1.0\r\n"
                              "Accept: text/html,application/xhtml+xml\r\n"
                              "\r\n";
        REQUIRE(testHeaders(headers, r));
        CHECK(r.headers.size() == 3);
#if 0
        CHECK(r["Host"] == "www.example.com");
        CHECK(r["User-Agent"] == "TestClient/1.0");
        CHECK(r["Accept"] == "text/html,application/xhtml+xml");
#endif
    }

    SUBCASE("Case-Insensitive Header Keys") {
        const char *headers = "hOsT: domain.org\r\n"
                              "CONTENT-length: 1234\r\n"
                              "\r\n";
        REQUIRE(testHeaders(headers, r));
        CHECK(r.headers.size() == 2);
#if 0
        CHECK(r["Host"] == "domain.org");
        CHECK(r["Content-Length"] == "1234");
#endif
    }

    SUBCASE("Whitespace around Header Values") {
        // Whitespace after the colon should be trimmed.
        const char *headers = "Connection:    keep-alive   \r\n"
                              "Cache-Control:  no-cache \r\n"
                              "\r\n";
        REQUIRE(testHeaders(headers, r));
#if 0
        CHECK(r.headers.size() == 2);
        CHECK(r["Connection"] == "keep-alive");
        CHECK(r["Cache-Control"] == "no-cache");
#endif
    }

    SUBCASE("Empty Header Section") {
        REQUIRE(testHeaders("\r\n", r));
        CHECK(r.headers.empty());
    }

    SUBCASE("Malformed or Invalid Headers") {
        CHECK_FALSE(testHeaders("Host www.example.com\r\n\r\n", r));    // Missing colon
        CHECK_FALSE(testHeaders("User-Agent: Test\r\nInvalid\r\n", r)); // Line without colon
        CHECK_FALSE(testHeaders("Header:\r\n\r\n", r));     // Empty value (some parsers allow this)
        CHECK_FALSE(testHeaders("Header: Value\n\r\n", r)); // Incorrect line ending
        CHECK_FALSE(testHeaders("Bad Header: Value\r\n\r\n", r)); // Space in header key
    }
}
