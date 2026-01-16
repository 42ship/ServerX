#include "doctest.h"

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

#include "../test_utils.hpp"
#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/RequestParser.hpp"
#include "http/Router.hpp"

using namespace http;
using namespace std;

static const int port = 9191;
static const char configF[] = "test_config/upload_test.conf";
static const char configDir[] = "test_config";

// ------------------------------------------------------------
// Test helpers
// ------------------------------------------------------------

static string makeConfig() {
    return string("server {\n"
                  "    listen localhost:9191;\n"
                  "    root test_www;\n"
                  "    max_body_size 5;\n"
                  "\n"
                  "    location / {\n"
                  "        index index.html;\n"
                  "    }\n"
                  "\n"
                  "    location /img/ {\n"
                  "        root img;\n"
                  "        upload_path uploads;\n"
                  "    }\n"
                  "}\n");
}

static string makeBody() { return "<html><body>data</body></html>"; }

static string makeRequestTo(const string &path, const string &headers) {
    return "POST " + path + " HTTP/1.1\r\n" + headers + "\r\n" + makeBody();
}

static void handleRequestParsingState(http::RequestParser::State state, const Router &router, Request &req, RequestParser &reqParser) {
    switch (state) {
    case http::RequestParser::ERROR:
        break;

    case http::RequestParser::HEADERS_READY:
        router.matchServerAndLocation(port, req);
        handleRequestParsingState(reqParser.proceedReadingBody(), router, req, reqParser);
        break;

    case http::RequestParser::REQUEST_READY:
        router.matchServerAndLocation(port, req);
        break;

    default:
        break; // WAITING_FOR_DATA
    }
}

static Request &makeUploadRequest(config::ServerConfig &conf, const MimeTypes &mime, Request &req,
                                  const string &path, const string &headers) {
    RequestParser parser(req, 100);
    Router router(conf, mime);
    string raw = makeRequestTo(path, headers);
    handleRequestParsingState(parser.feed(raw.c_str(), raw.size()), router, req, parser);

    return req;
}

// ------------------------------------------------------------
// RAII environment
// ------------------------------------------------------------

struct UploadTestEnv {
    UploadTestEnv(bool withUploadDir, mode_t uploadPerm) {
        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);

        if (withUploadDir) {
            mkdir("test_www/img/uploads", uploadPerm);
        }

        mkdir(configDir, 0777);
        writeFile(makeConfig(), configF);
    }

    UploadTestEnv() {
        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0777);

        mkdir(configDir, 0777);
        writeFile(makeConfig(), configF);
    }

    ~UploadTestEnv() {
        chmod("test_www/img/uploads", 0755);
        removeDirectoryRecursive("test_www");
        removeDirectoryRecursive(configDir);
    }
};

// ------------------------------------------------------------
// Filename header variants
// ------------------------------------------------------------

struct FilenameHeaderCase {
    const char *name;
    const char *header;
};

static const FilenameHeaderCase filenameHeaders[] = {
    {"X-Filename", "X-Filename: test.bin\r\n"},
    {"Content-Disposition", "Content-Disposition: filename=test.bin\r\n"}};

static const size_t filenameHeaderCount = sizeof(filenameHeaders) / sizeof(filenameHeaders[0]);

// ------------------------------------------------------------
// Tests
// ------------------------------------------------------------

TEST_CASE("POST /img returns 500 when upload directory is missing") {
    UploadTestEnv env(false, 0777);

    CHECK(access(configF, F_OK) == 0);

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    size_t i;
    for (i = 0; i < filenameHeaderCount; ++i) {
        SUBCASE(filenameHeaders[i].name) {
            Request req;
            makeUploadRequest(conf, mime, req, "/img/",
                              string(filenameHeaders[i].header) + "Host: localhost:9191\r\n"
                                                                  "Content-Length: 64\r\n"
                                                                  "Content-Type: text/html\r\n");

            Response res;
            FileUploadHandler::handle(req, res, mime);
            CHECK(res.status() == INTERNAL_SERVER_ERROR);
        }
    }
}

TEST_CASE("POST /img returns 413 when Content-Length exceeds max_body_size") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    size_t i;
    for (i = 0; i < filenameHeaderCount; ++i) {
        SUBCASE(filenameHeaders[i].name) {
            Request req;
            makeUploadRequest(conf, mime, req, "/img/",
                              string(filenameHeaders[i].header) + "Host: localhost:9191\r\n"
                                                                  "Content-Length: 999999\r\n"
                                                                  "Content-Type: text/html\r\n");
            Response res;
            FileUploadHandler::handle(req, res, mime);
            CHECK(res.status() == PAYLOAD_TOO_LARGE);
            cerr << req.path() << endl;
            cerr << req.uri() << endl;
        }
    }
}

TEST_CASE("POST / returns 405 when location has no upload_path") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    size_t i;
    for (i = 0; i < filenameHeaderCount; ++i) {
        SUBCASE(filenameHeaders[i].name) {
            Request req;
            makeUploadRequest(conf, mime, req, "/",
                              string(filenameHeaders[i].header) + "Host: localhost:9191\r\n"
                                                                  "Content-Length: 32\r\n"
                                                                  "Content-Type: text/html\r\n");

            Response res;
            FileUploadHandler::handle(req, res, mime);
            CHECK(res.status() == METHOD_NOT_ALLOWED);
        }
    }
}

TEST_CASE("POST /img returns 403 when upload directory is not writable") {
    UploadTestEnv env(true, 0555);
    mkdir(configDir, 0777);
    writeFile(makeConfig(), configF);

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    size_t i;
    for (i = 0; i < filenameHeaderCount; ++i) {
        SUBCASE(filenameHeaders[i].name) {
            Request req;
            makeUploadRequest(conf, mime, req, "/img/",
                              string(filenameHeaders[i].header) + "Host: localhost:9191\r\n"
                                                                  "Content-Length: 32\r\n"
                                                                  "Content-Type: text/html\r\n");

            Response res;
            FileUploadHandler::handle(req, res, mime);
            CHECK(res.status() == FORBIDDEN);
        }
    }
}

TEST_CASE("POST /img returns 411 when Content-Length header is missing") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    Request req;
    makeUploadRequest(conf, mime, req, "/img/",
                      "X-Filename: test.bin\r\n"
                      "Host: localhost:9191\r\n"
                      "Content-Type: text/html\r\n");

    Response res;
    FileUploadHandler::handle(req, res, mime);
    CHECK(res.status() == LENGTH_REQUIRED);
}

TEST_CASE("POST /img returns 415 on multipart/form-data") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    Request req;
    makeUploadRequest(conf, mime, req, "/img/",
                      "X-Filename: test.bin\r\n"
                      "Host: localhost:9191\r\n"
                      "Content-Length: 64\r\n"
                      "Content-Type: multipart/form-data\r\n");

    Response res;
    FileUploadHandler::handle(req, res, mime);
    CHECK(res.status() == UNSUPPORTED_MEDIA_TYPE);
}

TEST_CASE("POST /img returns 400 when filename header is missing") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    Request req;
    makeUploadRequest(conf, mime, req, "/img/",
                      "Host: localhost:9191\r\n"
                      "Content-Length: 64\r\n"
                      "Content-Type: application/octet-stream\r\n");

    Response res;
    FileUploadHandler::handle(req, res, mime);
    CHECK(res.status() == BAD_REQUEST);
}

TEST_CASE("POST /img uploads file and returns 201 with Location header") {
    UploadTestEnv env;

    config::ServerConfig conf(configF, false);
    MimeTypes mime;

    size_t i;
    for (i = 0; i < filenameHeaderCount; ++i) {
        SUBCASE(filenameHeaders[i].name) {
            Request req;
            makeUploadRequest(conf, mime, req, "/img/",
                              string(filenameHeaders[i].header) +
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 4\r\n"
                                  "Content-Type: application/octet-stream\r\n");

            Response res;
            FileUploadHandler::handle(req, res, mime);

            CHECK(res.status() == CREATED);
            CHECK(res.headers().has("Location"));
            CHECK(res.headers().get("Location").compare("/img/uploads/test.bin") == 0);
            CHECK(access("test_www/img/uploads/test.bin", F_OK) == 0);

            unlink("test_www/img/uploads/test.bin");
        }
    }
}
