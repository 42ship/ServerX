#include "doctest.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../TestableRequest.hpp"
#include "../test_utils.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/String.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

using namespace http;
using namespace std;
using namespace config;

// ----------------- helpers -----------------

static bool createFile(const char *path, const char *data = "x", size_t len = 1) {
    int fd = ::open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1)
        return false;
    ssize_t w = ::write(fd, data, len);
    ::close(fd);
    return (w == (ssize_t)len);
}

static LocationBlock createMockLocationWithUploadPath(const string &root, const string &path,
                                                      const string &uploadPath) {
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);
    ArgumentVector args;
    args.push_back(new config::String(uploadPath));
    loc.add("upload_path", args);
    return loc;
}

static LocationBlock createMockLocationWithoutUploadPath(const string &root, const string &path) {
    LocationBlock loc;
    loc.path(path);
    loc.add("root", root);
    return loc;
}

static ServerBlock createMockServer() {
    ServerBlock server;
    server.port(8080);
    server.address("localhost");
    return server;
}

static TestableRequest createDeleteRequest(const ServerBlock *server, const LocationBlock *location,
                                           const string &reqPath) {
    TestableRequest req;
    req.server(server);
    req.location(location);
    req.method(RequestStartLine::DELETE);
    req.path(reqPath);
    return req;
}

// ----------------- tests -----------------

TEST_CASE("DELETE - 204 No Content when deleting an existing file") {
    mkdir("test_www", 0777);
    mkdir("test_www/uploads", 0777);
    CHECK(createFile("test_www/uploads/to-delete.bin"));

    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/uploads/to-delete.bin");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == NO_CONTENT);
    CHECK(access("test_www/uploads/to-delete.bin", F_OK) == -1);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 404 Not Found when resource does not exist") {
    mkdir("test_www", 0777);
    mkdir("test_www/uploads", 0777);

    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/uploads/missing.bin");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == NOT_FOUND);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 403 Forbidden when parent directory is not writable") {
    mkdir("test_www", 0777);
    mkdir("test_www/uploads", 0777);
    CHECK(createFile("test_www/uploads/locked.bin"));
    CHECK(chmod("test_www/uploads", 0555) == 0);

    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/uploads/locked.bin");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == FORBIDDEN);

    chmod("test_www/uploads", 0755);
    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 409 Conflict when deleting non-empty directory") {
    mkdir("test_www", 0777);
    mkdir("test_www/uploads", 0777);
    mkdir("test_www/uploads/dir", 0777);
    CHECK(createFile("test_www/uploads/dir/file.txt"));

    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/uploads/dir");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == CONFLICT);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 204 No Content when deleting empty directory") {
    mkdir("test_www", 0777);
    mkdir("test_www/uploads", 0777);
    mkdir("test_www/uploads/emptydir", 0777);

    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/uploads/emptydir");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == NO_CONTENT);
    CHECK(access("test_www/uploads/emptydir", F_OK) == -1);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 405 Method Not Allowed when location has no upload_path") {
    mkdir("test_www", 0777);
    CHECK(createFile("test_www/file.txt"));

    LocationBlock loc = createMockLocationWithoutUploadPath("test_www", "/");
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, &loc, "/file.txt");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == METHOD_NOT_ALLOWED);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE - 404 when server is NULL") {
    LocationBlock loc = createMockLocationWithUploadPath("test_www", "/", "uploads");

    TestableRequest req = createDeleteRequest(nullptr, &loc, "/uploads/file.txt");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == NOT_FOUND);
}

TEST_CASE("DELETE - 404 when location is NULL") {
    ServerBlock server = createMockServer();

    TestableRequest req = createDeleteRequest(&server, nullptr, "/uploads/file.txt");
    Response res;

    FileDeleteHandler::handle(req, res);
    CHECK(res.status() == NOT_FOUND);
}
