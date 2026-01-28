#include "config/ServerConfig.hpp"
#include "doctest.h"
#include "http/Request.hpp"
#include <string>

TEST_CASE("AutoIndexDirective") {
    SUBCASE("Should enable autoindex") {
        const std::string configStr = R"(
server {
    listen 8080;
    location / {
        autoindex on;
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Request req;
        req.uri("/");
        const config::ServerBlock *server = sc.getServer(8080, req);
        REQUIRE(server != NULL);

        const config::LocationBlock *location = server->matchLocation(req);
        REQUIRE(location != NULL);
        CHECK(location->autoIndex() == true);
    }

    SUBCASE("Should disable autoindex") {
        const std::string configStr = R"(
server {
    listen 8080;
    location / {
        autoindex off;
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Request req;
        req.uri("/");
        const config::ServerBlock *server = sc.getServer(8080, req);
        REQUIRE(server != NULL);

        const config::LocationBlock *location = server->matchLocation(req);
        REQUIRE(location != NULL);
        CHECK(location->autoIndex() == false);
    }

    SUBCASE("Should default to off") {
        const std::string configStr = R"(
server {
    listen 8080;
    location / {
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Request req;
        req.uri("/");
        const config::ServerBlock *server = sc.getServer(8080, req);
        REQUIRE(server != NULL);
        const config::LocationBlock *location = server->matchLocation(req);
        CHECK(location->autoIndex() == false);
    }

    SUBCASE("Should throw on invalid value") {
        const std::string configStr = R"(
server {
    listen 8080;
    location / {
        autoindex maybe;
    }
}
        )";

        CHECK_THROWS(config::ServerConfig(configStr, false));
    }

    SUBCASE("Should throw when used in server block") {
        const std::string configStr = R"(
server {
    listen 8080;
    autoindex on;
}
        )";

        CHECK_THROWS(config::ServerConfig(configStr, false));
    }
}
