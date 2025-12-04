#include "http/Headers.hpp"
#include "doctest.h"
#include <sstream>

using namespace http;

TEST_CASE("Headers - Basic operations") {
    Headers headers;

    SUBCASE("Empty headers") {
        CHECK_FALSE(headers.has("content-type"));
        CHECK(headers.get("content-type").empty());
    }

    SUBCASE("Add and retrieve headers") {
        headers.add("Content-Type", "application/json");
        CHECK(headers.has("content-type"));
        CHECK(headers.get("Content-Type") == "application/json");
        CHECK(headers.get("content-type") == "application/json");
        CHECK(headers.get("CONTENT-TYPE") == "application/json");
    }

    SUBCASE("Case insensitive keys") {
        headers.add("Host", "example.com");
        CHECK(headers.get("host") == "example.com");
        CHECK(headers.get("HOST") == "example.com");
        CHECK(headers.get("HoSt") == "example.com");
    }

    SUBCASE("Overwrite existing header") {
        headers.add("Content-Type", "text/html");
        headers.add("Content-Type", "application/json");
        CHECK(headers.get("Content-Type") == "application/json");
    }

    SUBCASE("Multiple different headers") {
        headers.add("Host", "example.com");
        headers.add("User-Agent", "TestClient/1.0");
        headers.add("Accept", "text/html");
        CHECK(headers.has("host"));
        CHECK(headers.has("user-agent"));
        CHECK(headers.has("accept"));
    }

    SUBCASE("Empty key should be ignored") {
        headers.add("", "value");
        CHECK_FALSE(headers.has(""));
    }
}

TEST_CASE("Headers - Content-Length") {
    Headers headers;

    SUBCASE("No content-length header") {
        CHECK(headers.getContentLength() == 0);
    }

    SUBCASE("Valid content-length") {
        headers.add("Content-Length", "1234");
        CHECK(headers.getContentLength() == 1234);
    }

    SUBCASE("Content-length with leading/trailing spaces") {
        headers.add("Content-Length", " 5678 ");
        CHECK(headers.getContentLength() == 5678);
    }

    SUBCASE("Zero content-length") {
        headers.add("Content-Length", "0");
        CHECK(headers.getContentLength() == 0);
    }

    SUBCASE("Invalid content-length") {
        headers.add("Content-Length", "invalid");
        CHECK(headers.getContentLength() == 0);
    }
}

TEST_CASE("Headers - Chunked transfer encoding") {
    Headers headers;

    SUBCASE("No transfer-encoding header") {
        CHECK_FALSE(headers.isContentChunked());
    }

    SUBCASE("Transfer-encoding: chunked") {
        headers.add("Transfer-Encoding", "chunked");
        CHECK(headers.isContentChunked());
    }

    SUBCASE("Transfer-encoding with different case") {
        headers.add("transfer-encoding", "chunked");
        CHECK(headers.isContentChunked());
    }

    SUBCASE("Transfer-encoding: gzip") {
        // Note: Current implementation returns true for any Transfer-Encoding header
        // This is a known limitation (see TODO in implementation)
        headers.add("Transfer-Encoding", "gzip");
        CHECK(headers.isContentChunked()); // Currently returns true for any Transfer-Encoding
    }
}

TEST_CASE("Headers - Clear and erase") {
    Headers headers;
    headers.add("Host", "example.com");
    headers.add("Content-Type", "text/html");

    SUBCASE("Erase single header") {
        headers.erase("Host");
        CHECK_FALSE(headers.has("host"));
        CHECK(headers.has("content-type"));
    }

    SUBCASE("Clear all headers") {
        headers.clear();
        CHECK_FALSE(headers.has("host"));
        CHECK_FALSE(headers.has("content-type"));
    }

    SUBCASE("Erase non-existent header") {
        headers.erase("non-existent");
        CHECK(headers.has("host"));
        CHECK(headers.has("content-type"));
    }
}

TEST_CASE("Headers - toString serialization") {
    Headers headers;

    SUBCASE("Empty headers") {
        std::string result = headers.toString();
        CHECK(result.empty());
    }

    SUBCASE("Single header") {
        headers.add("Content-Type", "text/html");
        std::string result = headers.toString();
        CHECK(result.find("content-type: text/html\r\n") != std::string::npos);
    }

    SUBCASE("Multiple headers") {
        headers.add("Host", "example.com");
        headers.add("Content-Type", "application/json");
        std::string result = headers.toString();
        CHECK(result.find("host: example.com\r\n") != std::string::npos);
        CHECK(result.find("content-type: application/json\r\n") != std::string::npos);
    }
}

TEST_CASE("Headers - Parse from string") {
    Headers headers;

    SUBCASE("Valid headers with CRLF") {
        std::string input = "Host: example.com\r\n"
                           "Content-Type: text/html\r\n"
                           "\r\n";
        CHECK(Headers::parse(input, headers));
        CHECK(headers.get("host") == "example.com");
        CHECK(headers.get("content-type") == "text/html");
    }

    SUBCASE("Headers with whitespace") {
        std::string input = "Host:   example.com  \r\n"
                           "Content-Length:  1234\r\n"
                           "\r\n";
        CHECK(Headers::parse(input, headers));
        CHECK(headers.get("host") == "example.com");
        CHECK(headers.get("content-length") == "1234");
    }

    SUBCASE("Empty line terminates headers") {
        std::string input = "Host: example.com\r\n"
                           "\r\n"
                           "This should not be parsed\r\n";
        CHECK(Headers::parse(input, headers));
        CHECK(headers.has("host"));
    }

    SUBCASE("Missing colon in header") {
        std::string input = "Host example.com\r\n\r\n";
        CHECK_FALSE(Headers::parse(input, headers));
    }

    SUBCASE("Empty header key") {
        std::string input = ": value\r\n\r\n";
        CHECK_FALSE(Headers::parse(input, headers));
    }

    SUBCASE("Header without CRLF in strict mode") {
        std::string input = "Host: example.com\n\r\n";
        CHECK_FALSE(Headers::parse(input, headers, true));
    }

    SUBCASE("Header without CRLF in non-strict mode") {
        std::string input = "Host: example.com\n\n";
        CHECK(Headers::parse(input, headers, false));
    }
}

TEST_CASE("Headers - Parse from istringstream") {
    Headers headers;

    SUBCASE("Valid headers") {
        std::string input = "Host: example.com\r\n"
                           "User-Agent: TestClient\r\n"
                           "\r\n";
        std::istringstream stream(input);
        CHECK(Headers::parse(stream, headers));
        CHECK(headers.get("host") == "example.com");
        CHECK(headers.get("user-agent") == "TestClient");
    }

    SUBCASE("Empty stream") {
        std::string input = "\r\n";
        std::istringstream stream(input);
        CHECK(Headers::parse(stream, headers));
    }
}

TEST_CASE("Headers - Iterators") {
    Headers headers;
    headers.add("Host", "example.com");
    headers.add("Content-Type", "text/html");
    headers.add("Content-Length", "100");

    SUBCASE("Find existing header") {
        Headers::const_iterator it = headers.find("host");
        CHECK(it != headers.end());
        CHECK(it->second == "example.com");
    }

    SUBCASE("Find non-existent header") {
        Headers::const_iterator it = headers.find("non-existent");
        CHECK(it == headers.end());
    }

    SUBCASE("Iterate over all headers") {
        int count = 0;
        for (Headers::const_iterator it = headers.begin(); it != headers.end(); ++it) {
            count++;
        }
        CHECK(count == 3);
    }
}

TEST_CASE("Headers - getMap") {
    Headers headers;
    headers.add("Host", "example.com");
    headers.add("Content-Type", "text/html");

    const Headers::HeaderMap& map = headers.getMap();
    CHECK(map.size() == 2);
    CHECK(map.find("host") != map.end());
    CHECK(map.find("content-type") != map.end());
}
