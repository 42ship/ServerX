#include "http/internal/ChunkedBodyParser.hpp"
#include "doctest.h"
#include <string>

TEST_CASE("ChunkedBodyParser basic parsing") {
    http::ChunkedBodyParser parser;

    SUBCASE("Single chunk") {
        std::string input = "5\r\nHello\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "Hello");
        CHECK(consumed == input.size());
    }

    SUBCASE("Multiple chunks") {
        std::string input = "5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "Hello World");
        CHECK(consumed == input.size());
    }

    SUBCASE("Empty body (just final chunk)") {
        std::string input = "0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output.empty());
        CHECK(consumed == input.size());
    }

    SUBCASE("Uppercase hex") {
        std::string input = "A\r\n0123456789\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "0123456789");
    }

    SUBCASE("Lowercase hex") {
        std::string input = "a\r\n0123456789\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "0123456789");
    }
}

TEST_CASE("ChunkedBodyParser incremental parsing") {
    http::ChunkedBodyParser parser;

    SUBCASE("Partial chunk size") {
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse("5", 1, output, consumed);
        CHECK(status == http::ChunkedBodyParser::CHUNK_CONTINUE);

        status = parser.parse("\r\nHello\r\n0\r\n\r\n", 14, output, consumed);
        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "Hello");
    }

    SUBCASE("Partial chunk data") {
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse("5\r\nHel", 6, output, consumed);
        CHECK(status == http::ChunkedBodyParser::CHUNK_CONTINUE);
        CHECK(output == "Hel");

        status = parser.parse("lo\r\n0\r\n\r\n", 9, output, consumed);
        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "Hello");
    }

    SUBCASE("Byte by byte") {
        std::string input = "3\r\nabc\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;
        size_t pos = 0;

        while (pos < input.size()) {
            http::ChunkedBodyParser::Status status =
                parser.parse(input.c_str() + pos, 1, output, consumed);
            pos += consumed;
            if (pos < input.size()) {
                CHECK(status == http::ChunkedBodyParser::CHUNK_CONTINUE);
            } else {
                CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
            }
        }
        CHECK(output == "abc");
    }
}

TEST_CASE("ChunkedBodyParser error handling") {
    http::ChunkedBodyParser parser;

    SUBCASE("Invalid hex character") {
        std::string input = "XYZ\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_ERROR);
        CHECK(parser.errorStatus() == http::BAD_REQUEST);
    }

    SUBCASE("Missing CRLF after chunk data") {
        std::string input = "5\r\nHelloX";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_ERROR);
        CHECK(parser.errorStatus() == http::BAD_REQUEST);
    }
}

TEST_CASE("ChunkedBodyParser size limits") {
    http::ChunkedBodyParser parser;
    parser.setMaxBodySize(10);

    SUBCASE("Within limit") {
        std::string input = "5\r\nHello\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
        CHECK(output == "Hello");
    }

    SUBCASE("Exceeds limit in chunk size") {
        std::string input = "F\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_ERROR);
        CHECK(parser.errorStatus() == http::PAYLOAD_TOO_LARGE);
    }

    SUBCASE("Exceeds limit across chunks") {
        std::string input = "5\r\nHello\r\n6\r\n World\r\n0\r\n\r\n";
        std::string output;
        size_t consumed = 0;

        http::ChunkedBodyParser::Status status =
            parser.parse(input.c_str(), input.size(), output, consumed);

        CHECK(status == http::ChunkedBodyParser::CHUNK_ERROR);
        CHECK(parser.errorStatus() == http::PAYLOAD_TOO_LARGE);
    }
}

TEST_CASE("ChunkedBodyParser reset") {
    http::ChunkedBodyParser parser;

    std::string input = "5\r\nHello\r\n0\r\n\r\n";
    std::string output;
    size_t consumed = 0;

    http::ChunkedBodyParser::Status status =
        parser.parse(input.c_str(), input.size(), output, consumed);
    CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);

    parser.reset();
    output.clear();

    status = parser.parse(input.c_str(), input.size(), output, consumed);
    CHECK(status == http::ChunkedBodyParser::CHUNK_DONE);
    CHECK(output == "Hello");
}
