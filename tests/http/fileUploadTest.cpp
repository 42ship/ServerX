#include "doctest.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utils/Logger.hpp"

#include "../TestableRequest.hpp"
#include "../test_utils.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
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

static bool createTempBodyFile(const char* path, size_t size) {
    int fd = ::open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1)
        return false;

    string data(size, '0');
    ssize_t w = ::write(fd, data.c_str(), data.size());
    ::close(fd);

    return (w == (ssize_t)data.size());
}

static LocationBlock createUploadLocation(const string& root,
                                          const string& path,
                                          const string& uploadPath)
{
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);

    ArgumentVector args;
    args.push_back(new String(uploadPath));
    loc.add("upload_path", args);

    return loc;
}

static LocationBlock createNoUploadLocation(const string& root,
                                            const string& path)
{
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);
    return loc;
}

static ServerBlock createServer(size_t max_body_size, const LocationBlock &location) {
    ServerBlock server;
    server.port(8080);
    server.address("localhost");
    server.add("max_body_size", toString(max_body_size));
    server.maxBodySize();
    server.root("test_www");
    server.addLocation(location);
    return server;
}

static TestableRequest createUploadRequest(const ServerBlock* server,
                                           const LocationBlock* location,
                                           const string& uri,
                                           const string& filename,
                                           size_t contentLength,
                                           const string& bodyPath)
{
    TestableRequest req;
    req.server(server);
    req.location(location);
    req.method(RequestStartLine::POST);
    req.path(uri);
    req.headers().add("Content-Length", toString(contentLength));
    req.headers().add("Content-Type", "text/html");
    req.headers().add("X-Filename", filename);
    req.bodyPath(bodyPath);
    return req;
}

TEST_CASE("UPLOAD - 500 when upload directory is missing") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "uploads");
    ServerBlock server = createServer(5, loc);

    CHECK(createTempBodyFile("body.tmp", 4));

    TestableRequest req = createUploadRequest(
        &server, &loc, "/img/", "file.html", 4, "body.tmp");

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == INTERNAL_SERVER_ERROR);

    unlink("body.tmp");
    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 405 when location has no upload_path") {
    mkdir("test_www", 0777);

    LocationBlock loc = createNoUploadLocation("test_www", "/");
    ServerBlock server = createServer(0, loc);

    CHECK(createTempBodyFile("body.tmp", 4));

    TestableRequest req = createUploadRequest(
        &server, &loc, "/", "file.html", 4, "body.tmp");

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == METHOD_NOT_ALLOWED);

    unlink("body.tmp");
    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 403 when upload directory is not writable") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0555);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "img/uploads");
    ServerBlock server = createServer(5, loc);

    CHECK(createTempBodyFile("body.tmp", 4));

    TestableRequest req = createUploadRequest(
        &server, &loc, "/img/", "file.html", 4, "body.tmp");

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);
    CHECK(res.status() == FORBIDDEN);

    chmod("test_www/img/uploads", 0755);
    unlink("body.tmp");
    removeDirectoryRecursive("test_www");
}

TEST_CASE("UPLOAD - 201 Created on success with Location header") {
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    LocationBlock loc = createUploadLocation("test_www", "/img/", "img/uploads");
    ServerBlock server = createServer(5, loc);

    CHECK(createTempBodyFile("body.tmp", 4));

    TestableRequest req = createUploadRequest(
        &server, &loc, "/img/", "file.html", 4, "body.tmp");

    Response res;
    MimeTypes mime;

    FileUploadHandler::handle(req, res, mime);

    CHECK(res.status() == CREATED);
    CHECK(res.headers().has("Location"));
    CHECK(res.headers().get("Location") == "/img/uploads/file.html");
    CHECK(access("test_www/img/uploads/file.html", F_OK) == 0);

    unlink("test_www/img/uploads/file.html");
    unlink("body.tmp");
    removeDirectoryRecursive("test_www");
}
