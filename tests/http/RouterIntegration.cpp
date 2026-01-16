#include "config/ServerConfig.hpp"
#include "doctest.h"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Router.hpp"

TEST_CASE("Router: Integration with Regex Locations") {
    http::MimeTypes mime("config/mime.types");

    SUBCASE("Should match regex location over prefix location") {
        const std::string configStr = R"(
server {
    listen 9191;
    location / {
        root www/html;
    }
    location ~ \.php$ {
        cgi_pass /usr/bin/php-cgi;
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Router router(sc, mime);

        http::Request req;
        req.uri("/index.php");

        router.matchServerAndLocation(9191, req);

        const config::LocationBlock *loc = req.location();
        REQUIRE(loc != NULL);
        CHECK(loc->isRegex() == true);
        CHECK(loc->path() == "\\.php$");
        CHECK(loc->hasCgiPass() == true);
    }

    SUBCASE("Should respect declaration order for multiple regex matches") {
        const std::string configStr = R"(
server {
    listen 9191;
    location ~ \.py$ {
        cgi_pass /usr/bin/python2;
    }
    location ~ python/.*\.py$ {
        cgi_pass /usr/bin/python3;
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Router router(sc, mime);

        http::Request req;
        req.uri("/python/script.py");

        router.matchServerAndLocation(9191, req);

        const config::LocationBlock *loc = req.location();
        REQUIRE(loc != NULL);
        CHECK(loc->path() == "\\.py$");
    }

    SUBCASE("Prefix match should be used if no regex matches") {
        const std::string configStr = R"(
server {
    listen 9191;
    location /api/ {
        root www/api;
    }
    location ~ \.php$ {
        cgi_pass /usr/bin/php-cgi;
    }
}
        )";

        config::ServerConfig sc(configStr, false);
        http::Router router(sc, mime);

        http::Request req;
        req.uri("/api/data.json");

        router.matchServerAndLocation(9191, req);

        const config::LocationBlock *loc = req.location();
        REQUIRE(loc != NULL);
        CHECK(loc->isRegex() == false);
        CHECK(loc->path() == "/api/");
    }
}
