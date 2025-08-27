#include "doctest.h"
#include "config/ServerBlock.hpp"

using namespace config::details;

TEST_CASE("ServerNameMatcher: Exact and Case-Insensitive Matches") {
    std::vector<std::string> serverNames;
    serverNames.push_back("example.com");
    serverNames.push_back("www.example.org");

    SUBCASE("Should find an exact match") {
        CHECK(matchServerName(serverNames, "example.com") == true);
    }

    SUBCASE("Should perform a case-insensitive match") {
        CHECK(matchServerName(serverNames, "EXAMPLE.com") == true);
        CHECK(matchServerName(serverNames, "WwW.ExAmPlE.oRg") == true);
    }

    SUBCASE("Should not find a name that does not exist") {
        CHECK(matchServerName(serverNames, "test.com") == false);
    }
}

TEST_CASE("ServerNameMatcher: Wildcard Matches") {
    std::vector<std::string> serverNames;
    serverNames.push_back("*.example.com"); // Leading wildcard
    serverNames.push_back("www.example.*"); // Trailing wildcard

    SUBCASE("Leading wildcard should match subdomains") {
        CHECK(matchServerName(serverNames, "api.example.com") == true);
        CHECK(matchServerName(serverNames, "staging.api.example.com") == true);
        CHECK(matchServerName(serverNames, "WWW.EXAMPLE.COM") == true);
    }

    SUBCASE("Leading wildcard should NOT match the base domain") {
        CHECK(matchServerName(serverNames, "example.com") == false);
    }

    SUBCASE("Trailing wildcard should match different TLDs") {
        CHECK(matchServerName(serverNames, "www.example.com") == true);
        CHECK(matchServerName(serverNames, "www.example.org") == true);
        CHECK(matchServerName(serverNames, "www.example.net") == true);
    }

    SUBCASE("Trailing wildcard should NOT match subdomains") {
        CHECK(matchServerName(serverNames, "api.www.example.com") == false);
    }

    SUBCASE("Should not match if wildcard is in the middle") {
        // Assuming your logic doesn't support middle wildcards
        std::vector<std::string> middleWildcard;
        middleWildcard.push_back("www.*.com");
        CHECK(matchServerName(middleWildcard, "www.example.com") == false);
    }
}

TEST_CASE("ServerNameMatcher: Edge Cases") {
    std::vector<std::string> serverNames;
    serverNames.push_back("example.com");
    serverNames.push_back("_"); // Default server name

    SUBCASE("Should handle empty server name list") {
        std::vector<std::string> emptyList;
        CHECK(matchServerName(emptyList, "example.com") == false);
    }

    SUBCASE("Should handle empty host name string") {
        CHECK(matchServerName(serverNames, "") == false);
    }

    SUBCASE("Should treat '_' as a catch-all default (if implemented)") {
    }

    SUBCASE("Should not match partial names") {
        CHECK(matchServerName(serverNames, "ample.com") == false);
        CHECK(matchServerName(serverNames, "example.co") == false);
    }
}
