#include "doctest.h"

#include "../test_utils.hpp"
#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/RequestParser.hpp"
#include "http/Response.hpp"
#include "http/Router.hpp"

using namespace http;
using namespace std;

static const char *kConfPath = "config/return_test.conf";

// Helper to create a request with given path
static string makeRequestTo(const string &path, const string &headers) {
    return "GET " + path + " HTTP/1.1\r\n" + headers + "\r\n";
}

// Helper that builds a request and matches server & location
inline Request makeReturnRequest(config::ServerConfig &conf,
                                 MimeTypes &mime,
                                 const std::string &requestTo,
                                 const std::string &headers) {
    Request req;
    RequestParser parser(req, 8192);
    
    string rawRequest = makeRequestTo(requestTo, headers);
    parser.feed(rawRequest.c_str(), rawRequest.size());
    
    // Use Router to properly match server and location
    Router router(conf, mime);
    router.matchServerAndLocation(8080, req);
    
    return req;
}

TEST_CASE("ReturnHandler - Config loading") {
    REQUIRE_NOTHROW(config::ServerConfig conf(kConfPath, false));
}

TEST_CASE("ReturnHandler - Single argument (URL) defaults to 302") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-simple",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == FOUND); // 302
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/new-location");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 301 redirect with Location header") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-301",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == MOVED_PERMANENTLY); // 301
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/moved");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 302 redirect with Location header") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-302",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == FOUND); // 302
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/temp");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 303 redirect with Location header") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-303",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == SEE_OTHER); // 303
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/other");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 307 redirect with Location header") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-307",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == TEMPORARY_REDIRECT); // 307
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/temp307");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 308 redirect with Location header") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/redirect-308",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == PERMANENT_REDIRECT); // 308
        CHECK(response.headers().has("Location"));
        CHECK(response.headers().get("Location") == "http://example.com/perm308");
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 200 status with text body") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/return-200",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == OK); // 200
        CHECK_FALSE(response.headers().has("Location"));
        // Body should be set with "Success response"
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 404 status with text body") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/return-404",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == NOT_FOUND); // 404
        CHECK_FALSE(response.headers().has("Location"));
        // Body should be set with "Custom not found message"
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - 500 status with text body") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/return-500",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        CHECK(response.status() == INTERNAL_SERVER_ERROR); // 500
        CHECK_FALSE(response.headers().has("Location"));
        // Body should be set with "Custom error message"
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - Error when location has no return directive") {
    try {
        config::ServerConfig conf(kConfPath, false);
        MimeTypes mime;
        
        Request req = makeReturnRequest(conf, mime, "/no-return",
                                        "Host: localhost:8080\r\n");
        
        Response response;
        ReturnHandler::handle(req, response);
        
        // Should return 500 Internal Server Error
        CHECK(response.status() == INTERNAL_SERVER_ERROR);
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}

TEST_CASE("ReturnHandler - Error when server or location is NULL") {
    try {
        // Create a request without proper server/location setup
        Request req;
        RequestParser parser(req, 8192);
        string rawRequest = "GET /test HTTP/1.1\r\nHost: invalid\r\n\r\n";
        parser.feed(rawRequest.c_str(), rawRequest.size());
        
        // Don't set server/location (leave them NULL)
        Response response;
        ReturnHandler::handle(req, response);
        
        // Should return 404 Not Found (from CHECK_FOR_SERVER_AND_LOCATION macro)
        CHECK(response.status() == NOT_FOUND);
    } catch (const std::exception &e) {
        FAIL(e.what());
    }
}
