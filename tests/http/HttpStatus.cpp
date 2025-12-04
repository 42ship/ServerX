#include "http/HttpStatus.hpp"
#include "doctest.h"

using namespace http;

TEST_CASE("HttpStatus - toHttpStatus from integer") {
    SUBCASE("2xx Success codes") {
        CHECK(toHttpStatus(200) == OK);
        CHECK(toHttpStatus(201) == CREATED);
        CHECK(toHttpStatus(202) == ACCEPTED);
        CHECK(toHttpStatus(204) == NO_CONTENT);
    }

    SUBCASE("3xx Redirection codes") {
        CHECK(toHttpStatus(301) == MOVED_PERMANENTLY);
        CHECK(toHttpStatus(302) == FOUND);
        CHECK(toHttpStatus(303) == SEE_OTHER);
        CHECK(toHttpStatus(304) == NOT_MODIFIED);
    }

    SUBCASE("4xx Client error codes") {
        CHECK(toHttpStatus(400) == BAD_REQUEST);
        CHECK(toHttpStatus(401) == UNAUTHORIZED);
        CHECK(toHttpStatus(403) == FORBIDDEN);
        CHECK(toHttpStatus(404) == NOT_FOUND);
        CHECK(toHttpStatus(405) == METHOD_NOT_ALLOWED);
        CHECK(toHttpStatus(409) == CONFLICT);
        CHECK(toHttpStatus(411) == LENGTH_REQUIRED);
        CHECK(toHttpStatus(413) == PAYLOAD_TOO_LARGE);
        CHECK(toHttpStatus(415) == UNSUPPORTED_MEDIA_TYPE);
    }

    SUBCASE("5xx Server error codes") {
        CHECK(toHttpStatus(500) == INTERNAL_SERVER_ERROR);
        CHECK(toHttpStatus(501) == NOT_IMPLEMENTED);
        CHECK(toHttpStatus(502) == BAD_GATEWAY);
    }

    SUBCASE("Unknown status codes") {
        CHECK(toHttpStatus(0) == UNKNOWN_STATUS);
        CHECK(toHttpStatus(100) == UNKNOWN_STATUS);
        CHECK(toHttpStatus(999) == UNKNOWN_STATUS);
        CHECK(toHttpStatus(-1) == UNKNOWN_STATUS);
    }
}

TEST_CASE("HttpStatus - toHttpStatus from string") {
    SUBCASE("Valid string codes") {
        CHECK(toHttpStatus("200") == OK);
        CHECK(toHttpStatus("404") == NOT_FOUND);
        CHECK(toHttpStatus("500") == INTERNAL_SERVER_ERROR);
    }

    SUBCASE("Invalid string codes") {
        CHECK(toHttpStatus("") == UNKNOWN_STATUS);
        CHECK(toHttpStatus("abc") == UNKNOWN_STATUS);
        CHECK(toHttpStatus("999") == UNKNOWN_STATUS);
    }
}

TEST_CASE("HttpStatus - getReasonPhrase") {
    SUBCASE("2xx Success codes") {
        CHECK(std::string(getReasonPhrase(OK)) == "OK");
        CHECK(std::string(getReasonPhrase(CREATED)) == "Created");
        CHECK(std::string(getReasonPhrase(ACCEPTED)) == "Accepted");
        CHECK(std::string(getReasonPhrase(NO_CONTENT)) == "No Content");
    }

    SUBCASE("3xx Redirection codes") {
        CHECK(std::string(getReasonPhrase(MOVED_PERMANENTLY)) == "Moved Permanently");
        CHECK(std::string(getReasonPhrase(FOUND)) == "Found");
        CHECK(std::string(getReasonPhrase(SEE_OTHER)) == "See Other");
        CHECK(std::string(getReasonPhrase(NOT_MODIFIED)) == "Not Modified");
    }

    SUBCASE("4xx Client error codes") {
        CHECK(std::string(getReasonPhrase(BAD_REQUEST)) == "Bad Request");
        CHECK(std::string(getReasonPhrase(UNAUTHORIZED)) == "Unauthorized");
        CHECK(std::string(getReasonPhrase(FORBIDDEN)) == "Forbidden");
        CHECK(std::string(getReasonPhrase(NOT_FOUND)) == "Not Found");
        CHECK(std::string(getReasonPhrase(METHOD_NOT_ALLOWED)) == "Method Not Allowed");
        CHECK(std::string(getReasonPhrase(CONFLICT)) == "Conflict");
        CHECK(std::string(getReasonPhrase(LENGTH_REQUIRED)) == "Length Required");
        CHECK(std::string(getReasonPhrase(PAYLOAD_TOO_LARGE)) == "Payload Too Large");
        CHECK(std::string(getReasonPhrase(UNSUPPORTED_MEDIA_TYPE)) == "Unsupported Media Type");
    }

    SUBCASE("5xx Server error codes") {
        CHECK(std::string(getReasonPhrase(INTERNAL_SERVER_ERROR)) == "Internal Server Error");
        CHECK(std::string(getReasonPhrase(NOT_IMPLEMENTED)) == "Not Implemented");
        CHECK(std::string(getReasonPhrase(BAD_GATEWAY)) == "Bad Gateway");
    }

    SUBCASE("Unknown status code") {
        CHECK(std::string(getReasonPhrase(UNKNOWN_STATUS)) == "Unknown Status");
    }
}
