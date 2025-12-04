#include "http/Request.hpp"
#include "doctest.h"

using namespace http;

TEST_CASE("RequestStartLine - Method matching") {
    SUBCASE("Valid HTTP methods") {
        CHECK(RequestStartLine::matchHttpMethod("GET") == RequestStartLine::GET);
        CHECK(RequestStartLine::matchHttpMethod("POST") == RequestStartLine::POST);
        CHECK(RequestStartLine::matchHttpMethod("PUT") == RequestStartLine::PUT);
        CHECK(RequestStartLine::matchHttpMethod("DELETE") == RequestStartLine::DELETE);
    }

    SUBCASE("Invalid methods") {
        CHECK(RequestStartLine::matchHttpMethod("OPTIONS") == RequestStartLine::UNKNOWN);
        CHECK(RequestStartLine::matchHttpMethod("HEAD") == RequestStartLine::UNKNOWN);
        CHECK(RequestStartLine::matchHttpMethod("PATCH") == RequestStartLine::UNKNOWN);
        CHECK(RequestStartLine::matchHttpMethod("get") == RequestStartLine::UNKNOWN);
        CHECK(RequestStartLine::matchHttpMethod("") == RequestStartLine::UNKNOWN);
    }
}

TEST_CASE("RequestStartLine - Method to string") {
    CHECK(std::string(RequestStartLine::methodToString(RequestStartLine::GET)) == "GET");
    CHECK(std::string(RequestStartLine::methodToString(RequestStartLine::POST)) == "POST");
    CHECK(std::string(RequestStartLine::methodToString(RequestStartLine::PUT)) == "PUT");
    CHECK(std::string(RequestStartLine::methodToString(RequestStartLine::DELETE)) == "DELETE");
    CHECK(std::string(RequestStartLine::methodToString(RequestStartLine::UNKNOWN)) == "UNKNOWN");
}

TEST_CASE("RequestStartLine - Parse valid request lines") {
    SUBCASE("Simple GET request") {
        RequestStartLine rsl = RequestStartLine::parse("GET /index.html HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::GET);
        CHECK(rsl.uri == "/index.html");
        CHECK(rsl.path == "/index.html");
        CHECK(rsl.queryString == "");
        CHECK(rsl.version == "HTTP/1.1");
    }

    SUBCASE("POST request") {
        RequestStartLine rsl = RequestStartLine::parse("POST /api/users HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::POST);
        CHECK(rsl.uri == "/api/users");
        CHECK(rsl.path == "/api/users");
        CHECK(rsl.queryString == "");
    }

    SUBCASE("Request with query string") {
        RequestStartLine rsl = RequestStartLine::parse("GET /search?q=test&limit=10 HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::GET);
        CHECK(rsl.uri == "/search?q=test&limit=10");
        CHECK(rsl.path == "/search");
        CHECK(rsl.queryString == "q=test&limit=10");
        CHECK(rsl.version == "HTTP/1.1");
    }

    SUBCASE("DELETE request") {
        RequestStartLine rsl = RequestStartLine::parse("DELETE /users/123 HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::DELETE);
        CHECK(rsl.uri == "/users/123");
        CHECK(rsl.path == "/users/123");
    }

    SUBCASE("PUT request") {
        RequestStartLine rsl = RequestStartLine::parse("PUT /files/document.txt HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::PUT);
        CHECK(rsl.uri == "/files/document.txt");
    }

    SUBCASE("Root path") {
        RequestStartLine rsl = RequestStartLine::parse("GET / HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::GET);
        CHECK(rsl.uri == "/");
        CHECK(rsl.path == "/");
        CHECK(rsl.queryString == "");
    }

    SUBCASE("HTTP/1.0 version") {
        RequestStartLine rsl = RequestStartLine::parse("GET /page HTTP/1.0");
        CHECK(rsl.method == RequestStartLine::GET);
        CHECK(rsl.version == "HTTP/1.0");
    }

    SUBCASE("Complex query string") {
        RequestStartLine rsl = RequestStartLine::parse("GET /api?name=John%20Doe&age=30&active=true HTTP/1.1");
        CHECK(rsl.path == "/api");
        CHECK(rsl.queryString == "name=John%20Doe&age=30&active=true");
    }

    SUBCASE("Query string with question mark in value") {
        RequestStartLine rsl = RequestStartLine::parse("GET /search?q=what?how HTTP/1.1");
        CHECK(rsl.path == "/search");
        // Note: Only splits on first '?'
        CHECK(rsl.queryString == "q=what?how");
    }
}

TEST_CASE("RequestStartLine - Parse invalid request lines") {
    SUBCASE("Missing parts") {
        RequestStartLine rsl = RequestStartLine::parse("GET /path");
        CHECK(rsl.method == RequestStartLine::UNKNOWN);
    }

    SUBCASE("Empty string") {
        RequestStartLine rsl = RequestStartLine::parse("");
        CHECK(rsl.method == RequestStartLine::UNKNOWN);
    }

    SUBCASE("Only method") {
        RequestStartLine rsl = RequestStartLine::parse("GET");
        CHECK(rsl.method == RequestStartLine::UNKNOWN);
    }

    SUBCASE("Unknown method") {
        RequestStartLine rsl = RequestStartLine::parse("OPTIONS /path HTTP/1.1");
        CHECK(rsl.method == RequestStartLine::UNKNOWN);
        CHECK(rsl.uri == "/path");
        CHECK(rsl.version == "HTTP/1.1");
    }
}

TEST_CASE("RequestStartLine - Default construction") {
    RequestStartLine rsl;
    CHECK(rsl.method == RequestStartLine::UNKNOWN);
    CHECK(rsl.version == "HTTP/1.1");
    CHECK(rsl.uri.empty());
    CHECK(rsl.path.empty());
    CHECK(rsl.queryString.empty());
}
