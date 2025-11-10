#include "doctest.h"

#include <sys/stat.h>
#include <unistd.h>

#if 0
#include "../test_utils.hpp"
#include "config/ServerConfig.hpp"
#include "config/internal/ConfigException.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "utils/Logger.hpp"

using namespace http;
using namespace std;

// Simple struct to hold request + config pointers
struct UploadRequestContext {
    Request req;
    const config::ServerBlock *server;
    const config::LocationBlock *location;
};

const char *filename = "config/test.conf";

static string makeBody() {
    return string(
        "\r\n"
        "<!doctype html>"
        "<html lang=\"en\">"
        "<head>"
        "  <meta charset=\"utf-8\" />"
        "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\" />\r\n"
        "  <title>Hello World</title>\r\n"
        "  <style>\r\n"
        "    html, body { height: 100%; margin: 0; font-family: system-ui, -apple-system, Segoe "
        "UI, Roboto, Ubuntu, Cantarell, \"Helvetica Neue\", Arial, \"Noto Sans\", sans-serif; }\r\n"
        "    .wrap { display: grid; place-items: center; height: 100%; background: #f7f7fb; }\r\n"
        "    h1 { font-size: 3rem; margin: 0.2em 0; }\r\n"
        "    p { color: #555; margin: 0; }\r\n"
        "  </style>\r\n"
        "</head>\r\n"
        "<body>\r\n"
        "  <main class=\"wrap\">\r\n"
        "    <div>\r\n"
        "      <h1>Hello, World!</h1>\r\n"
        "      <p>It works 🎉</p>\r\n"
        "    </div>\r\n"
        "  </main>\r\n"
        "</body>\r\n"
        "</html>\r\n");
}

static string makeRequestTo(const string &path, const string &headers) {
    return "POST " + path + " HTTP/1.1\r\n" + headers + makeBody();
}

// Helper that builds a request and finds corresponding server & location
inline UploadRequestContext makeUploadRequest(config::ServerConfig &conf,
                                              const std::string &requestTo,
                                              const std::string &headers) {
    UploadRequestContext ctx;

    ctx.req = http::Request::parse(makeRequestTo(requestTo, headers));
    ctx.server = conf.getServer(9191, ctx.req.headers["Host"]);
    ctx.location = ctx.server->matchLocation(ctx.req.path);

    return ctx;
}

TEST_CASE("File uploading - 500 when upload dir is missing") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);
        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx = makeUploadRequest(conf, "/img/",
                                                         "X-Filename: test.html\r\n"
                                                         "Host: localhost:9191\r\n"
                                                         "Content-Length: 184467440737095516166\r\n"
                                                         "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == INTERNAL_SERVER_ERROR);
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/img/",
                                  "Content-Disposition: filename=test.html\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 184467440737095516166\r\n"
                                  "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == INTERNAL_SERVER_ERROR);
        }
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

TEST_CASE("File uploading - 413 when payload exceeds limit") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0777);

        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx = makeUploadRequest(conf, "/img/",
                                                         "X-Filename: test.html\r\n"
                                                         "Host: localhost:9191\r\n"
                                                         "Content-Length: 184467440737095516166\r\n"
                                                         "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == PAYLOAD_TOO_LARGE);
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/img/",
                                  "Content-Disposotion: filename=test.html\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 184467466\r\n"
                                  "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == PAYLOAD_TOO_LARGE);
        }

        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// 405: location / exists, but without upload_path → Method Not Allowed
TEST_CASE("File uploading - 405 on location without upload_path") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/www", 0777);

        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx = makeUploadRequest(conf, "/",
                                                         "X-Filename: index.html\r\n"
                                                         "Host: localhost:9191\r\n"
                                                         "Content-Length: 64\r\n"
                                                         "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == METHOD_NOT_ALLOWED);
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/",
                                  "Content-Disposition: filename=index.html\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 64\r\n"
                                  "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == METHOD_NOT_ALLOWED);
        }

        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// 403: directory exists, but without write permissions → FORBIDDEN
TEST_CASE("File uploading - 403 when no write permission on upload dir") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0555); // no write

        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx = makeUploadRequest(conf, "/img/",
                                                         "X-Filename: test.html\r\n"
                                                         "Host: localhost:9191\r\n"
                                                         "Content-Length: 64\r\n"
                                                         "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == FORBIDDEN);
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/img/",
                                  "Content-Disposition: filename=test.html\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 64\r\n"
                                  "Content-Type: text/html\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            CHECK(response.getStatus() == FORBIDDEN);
        }

        // restore permissions so cleanup works on some systems
        chmod("test_www/img/uploads", 0755);
        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// 411: without Content-Length and not chunked → LENGTH_REQUIRED (chunked currently not supported)
TEST_CASE("File uploading - 411 when Content-Length missing and not chunked") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0777);

        UploadRequestContext ctx = makeUploadRequest(conf, "/img/",
                                                     "X-Filename: test.html\r\n"
                                                     "Host: localhost:9191\r\n"
                                                     "Content-Type: text/html\r\n");
        Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
        CHECK(response.getStatus() == LENGTH_REQUIRED);

        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// // 415: multipart/form-data (currently not supported)
// TEST_CASE("File uploading - 415 on multipart/form-data") {
//     MimeTypes mime;
//     FileUploadHandler fileUpload(mime);

//     mkdir("test_www", 0777);
//     mkdir("test_www/img", 0777);
//     mkdir("test_www/img/uploads", 0777);

//     string headers = "X-Filename: logo.png\r\n"
//                      "Host: localhost:9191\r\n"
//                      "Content-Length: 64\r\n"
//                      "Content-Type: multipart/form-data\r\n";
//     Request req = Request::parse(getRequest(headers));

//     const config::ServerBlock *s = conf.getServer(9191, req.headers["Host"]);
//     const config::LocationBlock *l = s->getLocation(req.path);
//     Response response = fileUpload.handle(req, s, l);

//     CHECK(response.getStatus() == UNSUPPORTED_MEDIA_TYPE);
//     removeDirectoryRecursive("test_www");
// }

// 400: no X-Filename or Content-Disposition
TEST_CASE("File uploading - 400 when no filename provided") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0777);

        // no filename
        UploadRequestContext ctx = makeUploadRequest(conf, "/img/",
                                                     "Host: localhost:9191\r\n"
                                                     "Content-Length: 64\r\n"
                                                     "Content-Type: application/octet-stream\r\n");
        Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
        CHECK(response.getStatus() == BAD_REQUEST);

        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// 201: Success + check Location header
TEST_CASE("File uploading - 201 and Location header") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/img", 0777);
        mkdir("test_www/img/uploads", 0777);

        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/img/",
                                  "X-Filename: test.bin\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 4\r\n"
                                  "Content-Type: application/octet-stream\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            string location = ctx.location->path() + "uploads/" + "test.bin";

            CHECK(response.getStatus() == CREATED);
            CHECK(response.getHeaders().find("Location") != response.getHeaders().end());
            CHECK(response.getHeaders().at("Location") == location);
            CHECK(!access("test_www/img/uploads/test.bin", F_OK));

            unlink("test_www/img/uploads/test.bin");
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/img/",
                                  "Content-Disposition: filename=test.bin\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 4\r\n"
                                  "Content-Type: application/octet-stream\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            string location = ctx.location->path() + "uploads/" + "test.bin";

            CHECK(response.getStatus() == CREATED);
            CHECK(response.getHeaders().find("Location") != response.getHeaders().end());
            CHECK(response.getHeaders().at("Location") == location);
            CHECK(!access("test_www/img/uploads/test.bin", F_OK));
            unlink("test_www/img/uploads/test.bin");
        }

        removeDirectoryRecursive("test_www");
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}

// // 201: correction of extension under Content-Type: text/html
// TEST_CASE("File uploading - extension correction to .html") {
//     config::ServerConfig conf("config/test.conf");
//     MimeTypes mime;
//     FileUploadHandler fileUpload(mime);

//     mkdir("test_www", 0777);
//     mkdir("test_www/img", 0777);
//     mkdir("test_www/img/uploads", 0777);

//     // X-Filename without extension, Content-Type: text/html → expected page.html or page.htm
//     string headers =
//         "X-Filename: page\r\n"
//         "Host: localhost:9191\r\n"
//         "Content-Length: 64\r\n"
//         "Content-Type: text/html\r\n";
//     Request req = Request::parse(getRequest(headers));
//     const config::ServerBlock *s = conf.getServer(9191, req.headers["Host"]);
//     const config::LocationBlock *l = s->getLocation(req.path);

//     Response response = fileUpload.handle(req, s, l);
//     CHECK(response.getStatus() == CREATED);
//     CHECK((!access("test_www/img/uploads/page.html", F_OK)
//         || !access("test_www/img/uploads/page.htm", F_OK)));
//     CHECK(response.getHeaders().find("Location") != response.getHeaders().end());
//     CHECK((response.getHeaders().at("Location") == l->path() + "page.htm"));
//     unlink("test_www/img/uploads/page.html");
//     removeDirectoryRecursive("test_www");
// }

// 201: absolute upload_path for /upload/ (see. test.conf)
TEST_CASE("File uploading - absolute upload_path at /upload/") {
    try {
        config::ServerConfig conf(filename, false);
        MimeTypes mime;
        FileUploadHandler fileUpload(mime);

        mkdir("test_www", 0777);
        mkdir("test_www/upload", 0777); // absolute path in config points here

        SUBCASE("Custom X-Filename header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/upload/",
                                  "X-Filename: foo.bin\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 4\r\n"
                                  "Content-Type: application/octet-stream\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            string location = ctx.location->path() + "foo.bin";

            CHECK(response.getStatus() == CREATED);
            CHECK(response.getHeaders().find("Location") != response.getHeaders().end());
            CHECK(response.getHeaders().at("Location") == location);
            CHECK(!access("test_www/upload/foo.bin", F_OK));

            unlink("test_www/upload/foo.bin");
        }

        SUBCASE("Default Content-Disposition header check") {
            UploadRequestContext ctx =
                makeUploadRequest(conf, "/upload/",
                                  "Content-Disposition: filename=foo.bin\r\n"
                                  "Host: localhost:9191\r\n"
                                  "Content-Length: 4\r\n"
                                  "Content-Type: application/octet-stream\r\n");
            Response response = fileUpload.handle(ctx.req, ctx.server, ctx.location);
            string location = ctx.location->path() + "foo.bin";

            CHECK(response.getStatus() == CREATED);
            CHECK(response.getHeaders().find("Location") != response.getHeaders().end());
            CHECK(response.getHeaders().at("Location") == location);
            CHECK(!access("test_www/upload/foo.bin", F_OK));
            unlink("test_www/upload/foo.bin");

            removeDirectoryRecursive("test_www");
        }
    } catch (config::ConfigException const &e) {
        LOG_ERROR(e.what());
        return;
    } catch (const std::exception &e) {
        LOG_ERROR(e.what());
        return;
    }
}
#endif
