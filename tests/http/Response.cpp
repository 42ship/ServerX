#include "http/Response.hpp"
#include "doctest.h"
#include <fstream>
#include <cstdio>

using namespace http;

TEST_CASE("Response - Default construction") {
    Response response;

    SUBCASE("Default status is OK") {
        CHECK(response.status() == OK);
        CHECK(response.reasonPhrase() == "OK");
    }

    SUBCASE("Default protocol is HTTP/1.1") {
        CHECK(response.protocol() == "HTTP/1.1");
    }

    SUBCASE("No body by default") {
        CHECK(response.body() == nullptr);
    }
}

TEST_CASE("Response - Status management") {
    Response response;

    SUBCASE("Set status code") {
        response.status(NOT_FOUND);
        CHECK(response.status() == NOT_FOUND);
        CHECK(response.reasonPhrase() == "Not Found");
    }

    SUBCASE("Multiple status changes") {
        response.status(CREATED);
        CHECK(response.status() == CREATED);
        CHECK(response.reasonPhrase() == "Created");

        response.status(INTERNAL_SERVER_ERROR);
        CHECK(response.status() == INTERNAL_SERVER_ERROR);
        CHECK(response.reasonPhrase() == "Internal Server Error");
    }

    SUBCASE("Fluent interface") {
        Response &ref = response.status(OK);
        CHECK(&ref == &response);
    }
}

TEST_CASE("Response - Headers management") {
    Response response;

    SUBCASE("Add headers") {
        response.headers().add("Content-Type", "text/html");
        CHECK(response.headers().get("Content-Type") == "text/html");
    }

    SUBCASE("Headers are mutable") {
        response.headers().add("Host", "example.com");
        CHECK(response.headers().has("Host"));
        response.headers().erase("Host");
        CHECK_FALSE(response.headers().has("Host"));
    }

    SUBCASE("Const access to headers") {
        const Response &constResponse = response;
        response.headers().add("Test", "Value");
        CHECK(constResponse.headers().get("Test") == "Value");
    }
}

TEST_CASE("Response - Body management") {
    Response response;

    SUBCASE("Set no body") {
        response.headers().add("Content-Length", "100");
        response.headers().add("Content-Type", "text/html");
        
        response.setNoBody();
        
        CHECK(response.body() == nullptr);
        CHECK_FALSE(response.headers().has("Content-Length"));
        CHECK_FALSE(response.headers().has("Content-Type"));
    }

    SUBCASE("Set body in memory") {
        std::string data = "Hello, World!";
        response.setBodyInMemory(data, "text/plain");
        
        CHECK(response.body() != nullptr);
        CHECK(response.headers().get("Content-Type") == "text/plain");
        CHECK(response.headers().get("Content-Length") == "13");
    }

    SUBCASE("Set body from file") {
        // Create a temporary file
        const char* tmpFile = "/tmp/test_response_file.txt";
        std::ofstream out(tmpFile);
        out << "Test content";
        out.close();
        
        response.setBodyFromFile(tmpFile, "text/plain");
        
        CHECK(response.body() != nullptr);
        CHECK(response.headers().get("Content-Type") == "text/plain");
        // Content-Length should be set
        CHECK_FALSE(response.headers().get("Content-Length").empty());
        
        // Cleanup
        std::remove(tmpFile);
    }

    SUBCASE("Replace body") {
        response.setBodyInMemory("First", "text/plain");
        CHECK(response.headers().get("Content-Length") == "5");
        
        response.setBodyInMemory("Second content", "application/json");
        CHECK(response.headers().get("Content-Length") == "14");
        CHECK(response.headers().get("Content-Type") == "application/json");
    }

    SUBCASE("Fluent interface for body methods") {
        Response &ref1 = response.setNoBody();
        CHECK(&ref1 == &response);
        
        Response &ref2 = response.setBodyInMemory("data", "text/html");
        CHECK(&ref2 == &response);
    }
}

TEST_CASE("Response - Clear method") {
    Response response;
    
    response.status(NOT_FOUND);
    response.headers().add("Custom-Header", "value");
    response.setBodyInMemory("data", "text/plain");
    
    response.clear();
    
    CHECK(response.status() == OK);
    CHECK(response.reasonPhrase() == "OK");
    CHECK_FALSE(response.headers().has("Custom-Header"));
    CHECK(response.body() == nullptr);
}

TEST_CASE("Response - Build headers") {
    Response response;
    
    SUBCASE("Basic headers") {
        response.status(OK);
        response.headers().add("Content-Type", "text/html");
        response.headers().add("Content-Length", "100");
        
        std::vector<char> buffer;
        response.buildHeaders(buffer, true);
        
        std::string result(buffer.begin(), buffer.end());
        CHECK(result.find("HTTP/1.1 200 OK\r\n") != std::string::npos);
        CHECK(result.find("content-type: text/html\r\n") != std::string::npos);
        CHECK(result.find("content-length: 100\r\n") != std::string::npos);
        CHECK(result.find("\r\n\r\n") != std::string::npos);
    }

    SUBCASE("Build without body separator") {
        response.status(NO_CONTENT);
        
        std::vector<char> buffer;
        response.buildHeaders(buffer, false);
        
        std::string result(buffer.begin(), buffer.end());
        CHECK(result.find("HTTP/1.1 204 No Content\r\n") != std::string::npos);
        // Should not end with double CRLF
        CHECK(result.find("\r\n\r\n") == std::string::npos);
    }

    SUBCASE("Different status codes") {
        response.status(NOT_FOUND);
        
        std::vector<char> buffer;
        response.buildHeaders(buffer, true);
        
        std::string result(buffer.begin(), buffer.end());
        CHECK(result.find("HTTP/1.1 404 Not Found\r\n") != std::string::npos);
    }

    SUBCASE("Empty headers") {
        response.status(OK);
        
        std::vector<char> buffer;
        response.buildHeaders(buffer, true);
        
        std::string result(buffer.begin(), buffer.end());
        CHECK(result.find("HTTP/1.1 200 OK\r\n\r\n") != std::string::npos);
    }
}

TEST_CASE("Response - Fluent interface chaining") {
    Response response;
    
    response.status(CREATED)
        .headers().add("Content-Type", "application/json")
        .add("X-Custom", "value");
    
    CHECK(response.status() == CREATED);
    CHECK(response.headers().get("Content-Type") == "application/json");
    CHECK(response.headers().get("X-Custom") == "value");
}
