
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/ServerConfig.hpp"
#include "config/internal/ConfigException.hpp"
#include "doctest.h"
#include "http/Request.hpp"

const config::LocationBlock *testML(config::ServerBlock const &sb, std::string const &s) {
    http::Request req;
    req.uri(s);
    return sb.matchLocation(req);
}

const config::ServerBlock *testGS(config::ServerConfig const &sc, int port,
                                  std::string const &serverName) {
    http::Request req;
    req.headers().add("Host", serverName);
    return sc.getServer(port, req);
}

TEST_CASE("ServerConfig Integration") {
    SUBCASE("Should correctly parse a complete config file") {
        const std::string str = "server {\n"
                                "    listen 9191;\n"
                                "    location / {\n"
                                "        root website/html;\n"
                                "        index index.html;\n"
                                "    }\n"
                                "    location /img/ {\n"
                                "        root website/img;\n"
                                "        index logo.png favicon.ico;\n"
                                "    }\n"
                                "}\n";

        config::ServerConfig sc(str, false);
        // An empty server name should return the default server for the port
        const config::ServerBlock *sb = testGS(sc, 9191, "");

        REQUIRE(sb);
        CHECK(sb->port() == 9191);
        CHECK(sb->root() == ""); // Root is not defined at server level

        const config::LocationBlock *root_loc = testML(*sb, "/");
        REQUIRE(root_loc != NULL);
        CHECK(root_loc->root() == "website/html/");

        const config::LocationBlock *img_loc = testML(*sb, "/img/logo.png");
        REQUIRE(img_loc != NULL);
        CHECK(img_loc->root() == "website/img/");
    }

    SUBCASE("Should correctly combine server and location root paths") {
        const std::string str = "server {\n"
                                "    listen 8000;\n"
                                "    root /var/www/;\n"
                                "    location /site/ {\n"
                                "        root relative_path;\n"
                                "    }\n"
                                "    location /absolute/ {\n"
                                "        root /etc/nginx/;\n"
                                "    }\n"
                                "}\n";

        config::ServerConfig sc(str, false);
        const config::ServerBlock *sb = testGS(sc, 8000, "");

        REQUIRE(sb != NULL);
        CHECK(sb->root() == "/var/www/");

        const config::LocationBlock *relative_loc = testML(*sb, "/site/index.html");
        REQUIRE(relative_loc != NULL);
        CHECK(relative_loc->root() == "/var/www/relative_path/");

        const config::LocationBlock *absolute_loc = testML(*sb, "/absolute/config.txt");
        REQUIRE(absolute_loc != NULL);
        CHECK(absolute_loc->root() == "/etc/nginx/");
    }

    SUBCASE("Should throw on invalid semantic value in config file") {
        const std::string str = "server { listen 99999; }"; // Invalid port
        CHECK_THROWS_AS((config::ServerConfig(str, false)), const config::ConfigError &);
    }
}

TEST_CASE("Virtual Server Matching (Future Feature)") {
    SUBCASE("Should select server based on server_name") {
        config::ServerBlock f;
        f.port(8080).root("/var/www/first/");
        f.add("server_name", "first.com");
        f.add("server_name", "www.first.com");
        config::ServerBlock second;
        second.port(8080).root("/var/www/second/");
        f.add("server_name", "second.com");
        config::ServerConfig sc;
        sc.addServer(f);
        sc.addServer(second);

        const config::ServerBlock *sb_second = testGS(sc, 8080, "second.com");
        const config::ServerBlock *sb_first = testGS(sc, 8080, "www.first.com");
        const config::ServerBlock *sb_default = testGS(sc, 8080, "notfound.com");

        MESSAGE("This test is expected to fail until virtual server matching is implemented in "
                "ServerConfig::getServer.");
#if 0
        CHECK(sb_second->getRoot() == "/var/www/second/");
        CHECK(sb_first->getRoot() == "/var/www/first/");
        // The default server should be the first one defined for that port
        CHECK(sb_default->getRoot() == "/var/www/first/");
#endif
    }
}
