#include "doctest.h"

#include "config/Block.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/Integer.hpp"
#include "config/arguments/String.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

using namespace http;
using namespace std;
using namespace config;

// ============================= Test Helpers =====================================

/**
 * @brief Helper to create a mock LocationBlock with a return directive
 * @param status Optional status code (if 0, single-argument form is used)
 * @param value The URL or text value
 */
static LocationBlock createMockLocationWithReturn(int status, const string &value) {
    LocationBlock loc;
    loc.path("/test");
    
    if (status == 0) {
        // Single argument form: just URL
        ArgumentVector args;
        args.push_back(new config::String(value));
        loc.add("return", args);
    } else {
        // Two argument form: status code + URL/text
        ArgumentVector args;
        args.push_back(new config::Integer(status));
        args.push_back(new config::String(value));
        loc.add("return", args);
    }
    
    return loc;
}

/**
 * @brief Helper to create a mock ServerBlock
 */
static ServerBlock createMockServer() {
    ServerBlock server;
    server.port(8080);
    server.address("localhost");
    return server;
}

/**
 * @brief Helper to create a basic request with server and location set
 */
static Request createRequestWithMocks(const ServerBlock *server, const LocationBlock *location) {
    Request req;
    req.setServerBlock(server);
    req.setLocationBlock(location);
    return req;
}

// ============================= Handler Tests ====================================

TEST_CASE("ReturnHandler - Single argument (URL) defaults to 302") {
    LocationBlock loc = createMockLocationWithReturn(0, "http://example.com/new-location");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == FOUND); // 302
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/new-location");
}

TEST_CASE("ReturnHandler - 301 redirect with Location header") {
    LocationBlock loc = createMockLocationWithReturn(301, "http://example.com/moved");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == MOVED_PERMANENTLY); // 301
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/moved");
}

TEST_CASE("ReturnHandler - 302 redirect with Location header") {
    LocationBlock loc = createMockLocationWithReturn(302, "http://example.com/temp");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == FOUND); // 302
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/temp");
}

TEST_CASE("ReturnHandler - 303 redirect with Location header") {
    LocationBlock loc = createMockLocationWithReturn(303, "http://example.com/other");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == SEE_OTHER); // 303
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/other");
}

TEST_CASE("ReturnHandler - 307 redirect with Location header") {
    LocationBlock loc = createMockLocationWithReturn(307, "http://example.com/temp307");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == TEMPORARY_REDIRECT); // 307
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/temp307");
}

TEST_CASE("ReturnHandler - 308 redirect with Location header") {
    LocationBlock loc = createMockLocationWithReturn(308, "http://example.com/perm308");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == PERMANENT_REDIRECT); // 308
    CHECK(response.headers().has("Location"));
    CHECK(response.headers().get("Location") == "http://example.com/perm308");
}

TEST_CASE("ReturnHandler - 200 status with text body") {
    LocationBlock loc = createMockLocationWithReturn(200, "Success");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == OK); // 200
    CHECK_FALSE(response.headers().has("Location"));
    // Body should be set with "Success"
}

TEST_CASE("ReturnHandler - 404 status with text body") {
    LocationBlock loc = createMockLocationWithReturn(404, "NotFound");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == NOT_FOUND); // 404
    CHECK_FALSE(response.headers().has("Location"));
    // Body should be set with "NotFound"
}

TEST_CASE("ReturnHandler - 500 status with text body") {
    LocationBlock loc = createMockLocationWithReturn(500, "Error");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    CHECK(response.status() == INTERNAL_SERVER_ERROR); // 500
    CHECK_FALSE(response.headers().has("Location"));
    // Body should be set with "Error"
}

TEST_CASE("ReturnHandler - Error when location has no return directive") {
    LocationBlock loc; // No return directive
    loc.path("/test");
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    // Should return 500 Internal Server Error
    CHECK(response.status() == INTERNAL_SERVER_ERROR);
}

TEST_CASE("ReturnHandler - Error when server is NULL") {
    LocationBlock loc = createMockLocationWithReturn(301, "http://example.com");
    
    Request req = createRequestWithMocks(NULL, &loc);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    // Should return 404 Not Found (from CHECK_FOR_SERVER_AND_LOCATION macro)
    CHECK(response.status() == NOT_FOUND);
}

TEST_CASE("ReturnHandler - Error when location is NULL") {
    ServerBlock server = createMockServer();
    
    Request req = createRequestWithMocks(&server, NULL);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    // Should return 404 Not Found (from CHECK_FOR_SERVER_AND_LOCATION macro)
    CHECK(response.status() == NOT_FOUND);
}

TEST_CASE("ReturnHandler - Error when both server and location are NULL") {
    Request req = createRequestWithMocks(NULL, NULL);
    Response response;
    
    ReturnHandler::handle(req, response);
    
    // Should return 404 Not Found (from CHECK_FOR_SERVER_AND_LOCATION macro)
    CHECK(response.status() == NOT_FOUND);
}

