#include "config/ServerBlock.hpp"
#include "doctest.h"

using namespace config::details;

TEST_CASE("ServerNameMatcher: Exact and Case-Insensitive Matches") {
    std::vector<std::string> serverNames;
    serverNames.push_back("example.com");
    serverNames.push_back("www.example.org");

    SUBCASE("Should find an exact match") {
        CHECK(matchServerName(serverNames, "example.com") == true);
    }

#if 0
    SUBCASE("Should perform a case-insensitive match") {
        CHECK(matchServerName(serverNames, "EXAMPLE.com") == true);
        CHECK(matchServerName(serverNames, "WwW.ExAmPlE.oRg") == true);
    }
#endif

    SUBCASE("Should not find a name that does not exist") {
        CHECK(matchServerName(serverNames, "test.com") == false);
    }
}

TEST_CASE("ServerNameMatcher: Wildcard Matches") {
    std::vector<std::string> serverNames;
    serverNames.push_back("*.example.com"); // Leading wildcard
    serverNames.push_back("www.example.*"); // Trailing wildcard

#if 0
    SUBCASE("Leading wildcard should match subdomains") {
        CHECK(matchServerName(serverNames, "api.example.com") == true);
        CHECK(matchServerName(serverNames, "staging.api.example.com") == true);
        CHECK(matchServerName(serverNames, "WWW.EXAMPLE.COM") == true);
    }
#endif

    SUBCASE("Leading wildcard should NOT match the base domain") {
        CHECK(matchServerName(serverNames, "example.com") == false);
    }

#if 0
    SUBCASE("Trailing wildcard should match different TLDs") {
        CHECK(matchServerName(serverNames, "www.example.com") == true);
        CHECK(matchServerName(serverNames, "www.example.org") == true);
        CHECK(matchServerName(serverNames, "www.example.net") == true);
    }
#endif

    SUBCASE("Trailing wildcard should NOT match subdomains") {
        CHECK(matchServerName(serverNames, "api.www.example.com") == false);
    }

    SUBCASE("Should not match if wildcard is in the middle") {
        std::vector<std::string> middleWildcard;
        middleWildcard.push_back("www.*.com");
        CHECK(matchServerName(middleWildcard, "www.example.com") == false);
    }
}

TEST_CASE("ServerNameMatcher: Edge Cases") {
    std::vector<std::string> serverNames;
    serverNames.push_back("example.com");
    serverNames.push_back("_"); // Default server name

#if 0
    SUBCASE("Should handle empty server name list") {
        std::vector<std::string> emptyList;
        CHECK(matchServerName(emptyList, "example.com") == false);
    }
#endif

    SUBCASE("Should handle empty host name string") {
        CHECK(matchServerName(serverNames, "") == false);
    }

    SUBCASE("Should treat '_' as a catch-all default (if implemented)") {}

    SUBCASE("Should not match partial names") {
        CHECK(matchServerName(serverNames, "ample.com") == false);
        CHECK(matchServerName(serverNames, "example.co") == false);
    }
}
