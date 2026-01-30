#include "http/RequestParser.hpp"
#include "TestableRequest.hpp"
#include "doctest.h"
#include "utils/Logger.hpp"
#include <string>

using namespace http;

TEST_CASE("RequestParser - Header Parsing") {
    TestableRequest req;
    RequestParser parser(req, 1024);

    SUBCASE("Simple GET request") {
        std::string raw = "GET /index.html HTTP/1.1\r\nHost: localhost\r\n\r\n";
        parser.feed(raw.c_str(), raw.size());

        // For GET without body, it goes directly to REQUEST_READY in this implementation
        CHECK(parser.state() == RequestParser::REQUEST_READY);
        CHECK(req.method() == RequestStartLine::GET);
        CHECK(req.uri() == "/index.html");
        CHECK(req.headers().get("Host") == "localhost");
    }

    SUBCASE("Incremental header feeding") {
        parser.feed("GET / ", 6);
        CHECK(parser.state() == RequestParser::READING_HEADERS);

        parser.feed("HTTP/1.1\r\n", 10);
        CHECK(parser.state() == RequestParser::READING_HEADERS);

        parser.feed("Host: example.com\r\n\r\n", 21);
        // It goes directly to REQUEST_READY because no body is expected
        CHECK(parser.state() == RequestParser::REQUEST_READY);
        CHECK(req.uri() == "/");
    }

    SUBCASE("Header size limit exceeded") {
        RequestParser smallParser(req, 10);
        smallParser.feed("GET /verylonguri HTTP/1.1\r\n", 27);
        CHECK(smallParser.state() == RequestParser::ERROR);
        CHECK(smallParser.errorStatus() == BAD_REQUEST);
    }

    SUBCASE("Malformed request line") {
        parser.feed("INVALID_METHOD / HTTP/1.1\r\n\r\n", 29);
        CHECK(parser.state() == RequestParser::ERROR);
        CHECK(parser.errorStatus() == BAD_REQUEST);
    }
}

TEST_CASE("RequestParser - Body Parsing") {
    TestableRequest req;
    RequestParser parser(req, 1024);

    SUBCASE("POST with Content-Length") {
        std::string rawHeaders = "POST /upload HTTP/1.1\r\nContent-Length: 5\r\n\r\n";
        parser.feed(rawHeaders.c_str(), rawHeaders.size());

        CHECK(parser.state() == RequestParser::HEADERS_READY);

        parser.proceedReadingBody();
        CHECK(parser.state() == RequestParser::READING_BODY);

        std::string body = "hello";
        parser.feed(body.c_str(), body.size());
        CHECK(parser.state() == RequestParser::REQUEST_READY);
        REQUIRE(req.body() != static_cast<utils::TempFile *>(NULL));
        CHECK(req.bodyPath() != "");
    }

    SUBCASE("POST with chunked encoding") {
        std::string rawHeaders = "POST /chat HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n";
        parser.feed(rawHeaders.c_str(), rawHeaders.size());

        CHECK(parser.state() == RequestParser::HEADERS_READY);
        parser.proceedReadingBody();
        CHECK(parser.state() == RequestParser::READING_BODY);

        std::string chunk1 = "5\r\nHello\r\n";
        parser.feed(chunk1.c_str(), chunk1.size());
        CHECK(parser.state() == RequestParser::READING_BODY);

        std::string chunk2 = "6\r\n World\r\n";
        parser.feed(chunk2.c_str(), chunk2.size());
        CHECK(parser.state() == RequestParser::READING_BODY);

        std::string finalChunk = "0\r\n\r\n";
        parser.feed(finalChunk.c_str(), finalChunk.size());
        CHECK(parser.state() == RequestParser::REQUEST_READY);
    }
}
