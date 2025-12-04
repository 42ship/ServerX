#include "http/internal/ChunkedBodyParser.hpp"
#include "doctest.h"

using namespace http;

TEST_CASE("ChunkedBodyParser - Initial state") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    
    CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    CHECK(parser.errorStatus() == OK);
}

TEST_CASE("ChunkedBodyParser - Reset") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    parser.setMaxBodySize(1024);
    
    // Reset should return to initial state
    parser.reset();
    
    CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    CHECK(parser.errorStatus() == OK);
}

TEST_CASE("ChunkedBodyParser - Set max body size") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    
    SUBCASE("Set small limit") {
        parser.setMaxBodySize(100);
        CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    }
    
    SUBCASE("Set large limit") {
        parser.setMaxBodySize(10 * 1024 * 1024);
        CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    }
    
    SUBCASE("Set zero limit") {
        parser.setMaxBodySize(0);
        CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    }
}

TEST_CASE("ChunkedBodyParser - Feed data (TODO implementation)") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    
    SUBCASE("Empty buffer") {
        // Note: Current implementation returns DONE immediately
        ChunkedBodyParser::State result = parser.feed("");
        CHECK(result == ChunkedBodyParser::DONE);
    }
    
    SUBCASE("Valid chunk") {
        // Note: Full implementation is TODO
        ChunkedBodyParser::State result = parser.feed("5\r\nHello\r\n0\r\n\r\n");
        CHECK(result == ChunkedBodyParser::DONE);
    }
}

TEST_CASE("ChunkedBodyParser - Error handling") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    
    // Initially no error
    CHECK(parser.errorStatus() == OK);
    
    // After reset, still no error
    parser.reset();
    CHECK(parser.errorStatus() == OK);
}

TEST_CASE("ChunkedBodyParser - State transitions") {
    utils::TempFile tempFile;
    tempFile.open();
    
    ChunkedBodyParser parser(tempFile);
    
    SUBCASE("Initial state") {
        CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    }
    
    SUBCASE("State after feed (TODO implementation)") {
        // Note: Current implementation returns DONE but doesn't change state_
        ChunkedBodyParser::State result = parser.feed("test");
        CHECK(result == ChunkedBodyParser::DONE);
        // State remains PARSING_CHUNK_SIZE until implementation is complete
        CHECK(parser.state() == ChunkedBodyParser::PARSING_CHUNK_SIZE);
    }
}
