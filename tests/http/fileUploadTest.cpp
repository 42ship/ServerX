#include "doctest.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils/Logger.hpp"

#include "../TestableRequest.hpp"
#include "../test_utils.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/Integer.hpp"
#include "config/arguments/String.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Response.hpp"

using namespace http;
using namespace std;
using namespace config;

// ------------------------------------------------------------
// helpers
// ------------------------------------------------------------

static LocationBlock createUploadLocation(const string &root, const string &path,
                                          const string &uploadPath) {
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);
    loc.add("upload_path", uploadPath);

    return loc;
}

static LocationBlock createNoUploadLocation(const string &root, const string &path) {
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);
    return loc;
}

static ServerBlock createServer(size_t max_body_size, const LocationBlock &location) {
    ServerBlock server;
    server.port(8080);
    server.address("localhost");
    server.maxBodySize(max_body_size);

    server.root("test_www");
    server.addLocation(location);
    return server;
}

static void setupUploadRequest(TestableRequest &req, const string &uri, const string &filename,
                               size_t contentLength) {
    req.set(RequestStartLine::POST, uri);
    req.headers().add("Content-Length", toString(contentLength));
    req.headers().add("Content-Type", "text/html");
    req.headers().add("X-Filename", filename);

    utils::TempFile *tf = new utils::TempFile();
    if (tf->open()) {
        utils::writeFile(string(contentLength, '0'), tf->path().c_str());
        req.body(tf);
    } else {
        delete tf;
    }
}

TEST_CASE("UPLOAD - 500 when upload directory is missing") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "uploads");
    ServerBlock server = createServer(5, loc);

    TestableRequest req(&server, &loc);
    setupUploadRequest(req, "/img/", "file.html", 4);

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == INTERNAL_SERVER_ERROR);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 405 when location has no upload_path") {
    mkdir("test_www", 0777);

    LocationBlock loc = createNoUploadLocation("test_www", "/");
    ServerBlock server = createServer(0, loc);

    TestableRequest req(&server, &loc);
    setupUploadRequest(req, "/", "file.html", 4);

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == METHOD_NOT_ALLOWED);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 403 when upload directory is not writable") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0555);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "img/uploads");
    ServerBlock server = createServer(5, loc);

    TestableRequest req(&server, &loc);
    setupUploadRequest(req, "/img/", "file.html", 4);

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);
    CHECK(res.status() == FORBIDDEN);

    chmod("test_www/img/uploads", 0755);
    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 201 Created on success with Location header") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "img/uploads");
    ServerBlock server = createServer(5, loc);

    TestableRequest req(&server, &loc);
    setupUploadRequest(req, "/img/", "file.html", 4);

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == CREATED);
    CHECK(res.headers().has("Location"));
    CHECK(res.headers().get("Location") == "/img/uploads/file.html");
    CHECK(access("test_www/img/uploads/file.html", F_OK) == 0);

    unlink("test_www/img/uploads/file.html");
    removeDirectoryRecursive("test_www");
}
