#include "doctest.h"

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../test_utils.hpp"
#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/HttpRequest.hpp"
#include "http/MimeTypes.hpp"

using namespace http;
using namespace std;

static const char *kConfPath = "config/test.conf";

// ----------------- helpers -----------------

struct RequestContext {
    HttpRequest req;
    const config::ServerBlock *server;
    const config::LocationBlock *location;
};

static string makeRequestTo(const string &path, const string &headers) {
    return "DELETE " + path + " HTTP/1.1\r\n" + headers;
}

inline RequestContext makeDeleteRequest(config::ServerConfig &conf, const std::string &requestTo,
                                        const std::string &headers) {
    RequestContext ctx;
    ctx.req = http::HttpRequest::parse(makeRequestTo(requestTo, headers));
    ctx.server = conf.getServer(9191, ctx.req.headers["Host"]);
    ctx.location = ctx.server ? ctx.server->matchLocation(ctx.req.path) : NULL;
    return ctx;
}

static bool createFile(const char *path, const char *data = "x", size_t len = 1) {
    int fd = ::open(path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd == -1)
        return false;
    ssize_t w = ::write(fd, data, len);
    ::close(fd);
    return (w == (ssize_t)len);
}

// ----------------- tests -----------------

TEST_CASE("DELETE — 204 No Content when deleting an existing file") {
    config::ServerConfig conf(kConfPath, false);
    MimeTypes mime;
    FileDeleteHandler delHandler;

    // FS layout that matches test.conf: /img/ -> root test_www/img
    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    // Create file to delete
    CHECK(createFile("test_www/img/uploads/to-delete.bin"));

    RequestContext ctx = makeDeleteRequest(conf, "/img/uploads/to-delete.bin",
                                           "Host: localhost:9191\r\n"
                                           "Connection: close\r\n");

    HttpResponse res = delHandler.handle(ctx.req, ctx.server, ctx.location);
    CHECK(res.getStatus() == NO_CONTENT);
    CHECK(access("test_www/img/uploads/to-delete.bin", F_OK) == -1);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE — 404 Not Found when resource does not exist") {
    config::ServerConfig conf(kConfPath, false);
    MimeTypes mime;
    FileDeleteHandler delHandler;

    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    RequestContext ctx = makeDeleteRequest(conf, "/img/uploads/missing.bin",
                                           "Host: localhost:9191\r\n"
                                           "Connection: close\r\n");

    HttpResponse res = delHandler.handle(ctx.req, ctx.server, ctx.location);
    CHECK(res.getStatus() == NOT_FOUND);

    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE — 403 Forbidden when parent directory is not writable") {
    config::ServerConfig conf(kConfPath, false);
    MimeTypes mime;
    FileDeleteHandler delHandler;

    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    // File exists
    CHECK(createFile("test_www/img/uploads/locked.bin"));

    // Remove write permission from parent dir (uploads)
    CHECK(chmod("test_www/img/uploads", 0555) == 0);

    RequestContext ctx = makeDeleteRequest(conf, "/img/uploads/locked.bin",
                                           "Host: localhost:9191\r\n"
                                           "Connection: close\r\n");

    HttpResponse res = delHandler.handle(ctx.req, ctx.server, ctx.location);
    CHECK(res.getStatus() == FORBIDDEN);

    // Restore permissions for cleanup
    chmod("test_www/img/uploads", 0755);
    unlink("test_www/img/uploads/locked.bin");
    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE — 409 Conflict when deleting non-empty directory") {
    config::ServerConfig conf(kConfPath, false);
    MimeTypes mime;
    FileDeleteHandler delHandler;

    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    // Create non-empty directory
    mkdir("test_www/img/uploads/dir", 0777);
    CHECK(createFile("test_www/img/uploads/dir/file.txt"));

    RequestContext ctx = makeDeleteRequest(conf, "/img/uploads/dir",
                                           "Host: localhost:9191\r\n"
                                           "Connection: close\r\n");

    HttpResponse res = delHandler.handle(ctx.req, ctx.server, ctx.location);
    CHECK(res.getStatus() == CONFLICT);

    // cleanup
    unlink("test_www/img/uploads/dir/file.txt");
    rmdir("test_www/img/uploads/dir");
    removeDirectoryRecursive("test_www");
}

TEST_CASE("DELETE — 204 No Content when deleting empty directory") {
    config::ServerConfig conf(kConfPath, false);
    MimeTypes mime;
    FileDeleteHandler delHandler;

    mkdir("test_www", 0777);
    mkdir("test_www/img", 0777);
    mkdir("test_www/img/uploads", 0777);

    // Create empty directory
    mkdir("test_www/img/uploads/emptydir", 0777);

    RequestContext ctx = makeDeleteRequest(conf, "/img/uploads/emptydir",
                                           "Host: localhost:9191\r\n"
                                           "Connection: close\r\n");

    HttpResponse res = delHandler.handle(ctx.req, ctx.server, ctx.location);
    CHECK(res.getStatus() == NO_CONTENT);

    // directory should be gone
    CHECK(access("test_www/img/uploads/emptydir", F_OK) == -1);

    removeDirectoryRecursive("test_www");
}
