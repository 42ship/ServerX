#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "doctest.h"
#include "http/Request.hpp"
#include <map>
#include <string>

using namespace config;

// --- Test Fixture Setup ---
// This struct creates a pre-configured map of locations for our tests.
struct LocationTestFixture {
    std::map<std::string, LocationBlock> locations;

    LocationTestFixture() {
        LocationBlock root;
        root.path("/");
        root.add("root", "/var/www/html");
        locations[root.path()] = root;

        LocationBlock images;
        images.path("/images/");
        images.add("root", "/var/www/data/images");
        locations[images.path()] = images;

        LocationBlock imagesJpg;
        imagesJpg.path("/images/jpg/");
        imagesJpg.add("root", "/var/www/data/images/jpg");
        locations[imagesJpg.path()] = imagesJpg;
    }
};

TEST_CASE("LocationMatcher: Basic Matches") {
    LocationTestFixture fixture;
    const std::map<std::string, LocationBlock> &locs = fixture.locations;

    SUBCASE("Should find an exact match for a directory") {
        const LocationBlock *result = details::bestMatchLocation(locs, "/images/");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/images/");
    }

    SUBCASE("Should find a prefix match for a file inside a directory") {
        const LocationBlock *result = details::bestMatchLocation(locs, "/images/logo.png");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/images/");
    }

    SUBCASE("Should fall back to the root location '/' for unmatched paths") {
        const LocationBlock *result = details::bestMatchLocation(locs, "/some/other/path.html");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/");
    }

    SUBCASE("Should match the root location '/' exactly") {
        const LocationBlock *result = details::bestMatchLocation(locs, "/");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/");
    }
}

TEST_CASE("LocationMatcher: Longest Prefix Matching") {
    LocationTestFixture fixture;
    const std::map<std::string, LocationBlock> &locs = fixture.locations;

    SUBCASE("Should select the most specific (longest) matching prefix") {
        // This request matches "/", "/images/", and "/images/jpg/"
        // The longest match, "/images/jpg/", must be chosen.
        const LocationBlock *result = details::bestMatchLocation(locs, "/images/jpg/summer.jpeg");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/images/jpg/");
        CHECK(result->root() == "/var/www/data/images/jpg");
    }

    SUBCASE("Should select '/images/' over '/'") {
        const LocationBlock *result = details::bestMatchLocation(locs, "/images/png/winter.png");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/images/");
    }
}

TEST_CASE("LocationMatcher: Edge Cases") {
    LocationTestFixture fixture;
    const std::map<std::string, LocationBlock> &locs = fixture.locations;

    SUBCASE("Should return NULL if no locations are configured") {
        std::map<std::string, LocationBlock> emptyLocs;
        const LocationBlock *result = details::bestMatchLocation(emptyLocs, "/any/path");
        CHECK(result == NULL);
    }

    SUBCASE("Should return NULL if no root '/' fallback exists for an unmatched path") {
        std::map<std::string, LocationBlock> noRootLocs;
        noRootLocs["/data/"] = LocationBlock(); // A map with no "/" location

        const LocationBlock *result = details::bestMatchLocation(noRootLocs, "/unmatched/path");
        CHECK(result == NULL);
    }

    SUBCASE("Should handle an empty request URI by matching the root") {
        const LocationBlock *result = details::bestMatchLocation(locs, "");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/");
    }

    SUBCASE("Should correctly handle paths without a trailing slash") {
        // A request for "/images" does NOT match "location /images/".
        // It will be shortened to "/" and match the root location.
        const LocationBlock *result = details::bestMatchLocation(locs, "/images");
        REQUIRE(result != NULL);
        CHECK(result->path() == "/");
    }
}

TEST_CASE("LocationMatcher: Regex Matching") {
    ServerBlock server;

    LocationBlock php;
    php.path("\\.php$").matchType(config::LocationBlock::EXTENSION).extension(".php");

    php.add("cgi_pass", "/usr/bin/php-cgi");
    server.addLocation(php);

    LocationBlock py;
    py.path("\\.py$").matchType(config::LocationBlock::EXTENSION).extension(".py");

    py.add("cgi_pass", "/usr/bin/python3");
    server.addLocation(py);

    LocationBlock root;
    root.path("/");
    root.add("root", "/var/www/html");
    server.addLocation(root);

    SUBCASE("Should match .php regex") {
        http::Request req;
        req.uri("/index.php");
        const LocationBlock *result = server.matchLocation(req);
        REQUIRE(result != NULL);
        CHECK(result->isRegex() == true);
        CHECK(result->path() == "\\.php$");
    }

    SUBCASE("Should match .py regex") {
        http::Request req;
        req.uri("/script.py");
        const LocationBlock *result = server.matchLocation(req);
        REQUIRE(result != NULL);
        CHECK(result->isRegex() == true);
        CHECK(result->path() == "\\.py$");
    }

    SUBCASE("Should fallback to prefix match if no regex matches") {
        http::Request req;
        req.uri("/style.css");
        const LocationBlock *result = server.matchLocation(req);
        REQUIRE(result != NULL);
        CHECK(result->matchType() == config::LocationBlock::PREFIX);
        CHECK(result->path() == "/");
    }

    SUBCASE("Extension match priority: first declared extension should match first") {
        ServerBlock s2;
        config::LocationBlock py1;
        py1.path(".*\\.py$").matchType(config::LocationBlock::EXTENSION).extension(".py");
        py1.add("id", "1");
        s2.addLocation(py1);

        config::LocationBlock py2;
        py2.path("\\.py$").matchType(config::LocationBlock::EXTENSION).extension(".py");
        py2.add("id", "2");
        s2.addLocation(py2);

        http::Request req;
        req.uri("/test.py");
        const LocationBlock *result = s2.matchLocation(req);
        REQUIRE(result != NULL);
        CHECK(result->getFirstRawValue("id") == "1");
    }
}
