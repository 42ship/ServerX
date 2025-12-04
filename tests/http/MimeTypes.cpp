#include "http/MimeTypes.hpp"
#include "doctest.h"
#include <stdexcept>

using namespace http;

TEST_CASE("MimeTypes - Get MIME type by extension") {
    MimeTypes mimes(MIME_TYPES_PATH);

    SUBCASE("Text formats") {
        CHECK(mimes.getMimeType("html") == "text/html");
        CHECK(mimes.getMimeType("htm") == "text/html");
        CHECK(mimes.getMimeType("txt") == "text/plain");
        CHECK(mimes.getMimeType("css") == "text/css");
        CHECK(mimes.getMimeType("csv") == "text/csv");
        CHECK(mimes.getMimeType("md") == "text/markdown");
    }

    SUBCASE("Application formats") {
        CHECK(mimes.getMimeType("json") == "application/json");
        CHECK(mimes.getMimeType("xml") == "application/xml");
        CHECK(mimes.getMimeType("js") == "application/javascript");
        CHECK(mimes.getMimeType("mjs") == "application/javascript");
        CHECK(mimes.getMimeType("pdf") == "application/pdf");
    }

    SUBCASE("Image formats") {
        CHECK(mimes.getMimeType("png") == "image/png");
        CHECK(mimes.getMimeType("jpg") == "image/jpeg");
        CHECK(mimes.getMimeType("jpeg") == "image/jpeg");
        CHECK(mimes.getMimeType("gif") == "image/gif");
        CHECK(mimes.getMimeType("svg") == "image/svg+xml");
        CHECK(mimes.getMimeType("webp") == "image/webp");
        CHECK(mimes.getMimeType("ico") == "image/x-icon");
        CHECK(mimes.getMimeType("bmp") == "image/bmp");
    }

    SUBCASE("Archive formats") {
        CHECK(mimes.getMimeType("zip") == "application/zip");
        CHECK(mimes.getMimeType("tar") == "application/x-tar");
        CHECK(mimes.getMimeType("gz") == "application/gzip");
    }

    SUBCASE("Unknown extension") {
        CHECK(mimes.getMimeType("unknown") == "text/plain");
        CHECK(mimes.getMimeType("xyz") == "text/plain");
        CHECK(mimes.getMimeType("") == "text/plain");
    }
}

TEST_CASE("MimeTypes - Get extension by MIME type") {
    MimeTypes mimes(MIME_TYPES_PATH);

    SUBCASE("Text formats") {
        // Note: getMimeExt returns the last extension parsed from mime.types
        // For "text/html" with "html htm", it returns "htm" (the last one)
        CHECK(mimes.getMimeExt("text/html") == "htm");
        CHECK(mimes.getMimeExt("text/plain") == "txt");
        CHECK(mimes.getMimeExt("text/css") == "css");
    }

    SUBCASE("Application formats") {
        CHECK(mimes.getMimeExt("application/json") == "json");
        CHECK(mimes.getMimeExt("application/xml") == "xml");
    }

    SUBCASE("Image formats") {
        CHECK(mimes.getMimeExt("image/png") == "png");
        // Note: For "image/jpeg" with "jpg jpeg", it returns "jpeg" (the last one)
        CHECK(mimes.getMimeExt("image/jpeg") == "jpeg");
        CHECK(mimes.getMimeExt("image/gif") == "gif");
    }

    SUBCASE("Unknown MIME type") {
        CHECK(mimes.getMimeExt("unknown/type") == "txt");
        CHECK(mimes.getMimeExt("") == "txt");
    }
}

TEST_CASE("MimeTypes - Copy constructor") {
    MimeTypes mimes1(MIME_TYPES_PATH);
    MimeTypes mimes2(mimes1);

    CHECK(mimes2.getMimeType("html") == "text/html");
    CHECK(mimes2.getMimeType("json") == "application/json");
    CHECK(mimes2.getMimeExt("text/html") == "htm");
}

TEST_CASE("MimeTypes - Assignment operator") {
    MimeTypes mimes1(MIME_TYPES_PATH);
    MimeTypes mimes2 = mimes1;

    CHECK(mimes2.getMimeType("html") == "text/html");
    CHECK(mimes2.getMimeType("json") == "application/json");
    CHECK(mimes2.getMimeExt("text/html") == "htm");
}

TEST_CASE("MimeTypes - Invalid file path") {
    CHECK_THROWS_AS(MimeTypes("/path/that/does/not/exist.types"), std::runtime_error);
}
