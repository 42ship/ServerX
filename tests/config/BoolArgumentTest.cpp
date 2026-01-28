#include "config/arguments/Bool.hpp"
#include "config/internal/ConfigException.hpp"
#include "doctest.h"
#include "http/Request.hpp"

TEST_CASE("BoolArgument") {
    SUBCASE("True values") {
        config::Bool b1("true");
        CHECK(b1.getValue() == true);
        config::Bool b2("on");
        CHECK(b2.getValue() == true);
        config::Bool b3("yes");
        CHECK(b3.getValue() == true);
        config::Bool b4("1");
        CHECK(b4.getValue() == true);
    }

    SUBCASE("False values") {
        config::Bool b1("false");
        CHECK(b1.getValue() == false);
        config::Bool b2("off");
        CHECK(b2.getValue() == false);
        config::Bool b3("no");
        CHECK(b3.getValue() == false);
        config::Bool b4("0");
        CHECK(b4.getValue() == false);
    }

    SUBCASE("Mixed case") {
        config::Bool b1("True");
        CHECK(b1.getValue() == true);
        config::Bool b2("OFF");
        CHECK(b2.getValue() == false);
    }

    SUBCASE("Invalid values") {
        CHECK_THROWS_AS(config::Bool("invalid"), config::ConfigError);
        CHECK_THROWS_AS(config::Bool(""), config::ConfigError);
    }

    SUBCASE("Evaluation") {
        http::Request req; // Mock request if needed, though Bool doesn't use it
        config::Bool b1(true);
        CHECK(b1.evaluate(req) == "true");
        config::Bool b2(false);
        CHECK(b2.evaluate(req) == "false");
    }
}
