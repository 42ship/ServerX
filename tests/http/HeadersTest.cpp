#include "http/Headers.hpp"
#include "doctest.h"
#include <string>
#include <vector>

using namespace http;

TEST_CASE("Headers Class - Set-Cookie Handling (Multimap)") {
    Headers h;

    SUBCASE("Add single Set-Cookie") {
        h.add("Set-Cookie", "sessionID=123; Path=/");
        CHECK(h.get("Set-Cookie") == "sessionID=123; Path=/");
    }

    SUBCASE("Add multiple Set-Cookie (Multimap behavior)") {
        h.add("Set-Cookie", "sessionID=123; Path=/");
        h.add("Set-Cookie", "theme=dark; Path=/");

        std::vector<std::string> cookies = h.getAll("Set-Cookie");
        REQUIRE(cookies.size() == 2);
        CHECK(cookies[0] == "sessionID=123; Path=/");
        CHECK(cookies[1] == "theme=dark; Path=/");

        std::string serialized = h.toString();
        // toString() will now naturally have two lines for set-cookie
        CHECK(serialized.find("set-cookie: sessionID=123; Path=/\r\n") != std::string::npos);
        CHECK(serialized.find("set-cookie: theme=dark; Path=/\r\n") != std::string::npos);
    }

    SUBCASE("Set method (Overwrite behavior)") {
        h.add("X-Test", "val1");
        h.add("X-Test", "val2");
        h.set("X-Test", "finalValue");

        CHECK(h.get("X-Test") == "finalValue");
        CHECK(h.getAll("X-Test").size() == 1);
    }

    SUBCASE("Case insensitivity and has()") {
        h.add("Content-Type", "text/html");
        CHECK(h.has("content-type"));
        CHECK(h.get("CONTENT-TYPE") == "text/html");
    }

    SUBCASE("Erase method") {
        h.add("User-Agent", "Test");
        h.add("User-Agent", "Test2");
        h.erase("User-Agent");
        CHECK_FALSE(h.has("User-Agent"));
        CHECK(h.getAll("User-Agent").empty());
    }
}
