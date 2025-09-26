
#include "config/ConfigException.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/ServerConfig.hpp"
#include "doctest.h"

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
        const config::ServerBlock *sb = sc.getServer(9191, "");

        REQUIRE(sb);
        CHECK(sb->getPort() == 9191);
        CHECK(sb->getRoot() == ""); // Root is not defined at server level

        const config::LocationBlock *root_loc = sb->getLocation("/");
        REQUIRE(root_loc != NULL);
        CHECK(root_loc->getRoot() == "website/html/");

        const config::LocationBlock *img_loc = sb->getLocation("/img/logo.png");
        REQUIRE(img_loc != NULL);
        CHECK(img_loc->getRoot() == "website/img/");
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
        const config::ServerBlock *sb = sc.getServer(8000, "");

        REQUIRE(sb != NULL);
        CHECK(sb->getRoot() == "/var/www/");

        const config::LocationBlock *relative_loc = sb->getLocation("/site/index.html");
        REQUIRE(relative_loc != NULL);
        CHECK(relative_loc->getRoot() == "/var/www/relative_path/");

        const config::LocationBlock *absolute_loc = sb->getLocation("/absolute/config.txt");
        REQUIRE(absolute_loc != NULL);
        CHECK(absolute_loc->getRoot() == "/etc/nginx/");
    }

    SUBCASE("Should throw on invalid semantic value in config file") {
        const std::string str = "server { listen 99999; }"; // Invalid port
        CHECK_THROWS_AS((config::ServerConfig(str, false)), const config::ConfigError &);
    }
}

TEST_CASE("Virtual Server Matching (Future Feature)") {
    SUBCASE("Should select server based on server_name") {
        config::ServerBlock f;
        f.setListen("8080");
        f["server_name"] = {"first.com", "www.first.com"};
        f.setRoot("root /var/www/first/");
        config::ServerBlock second;
        second.setListen("8080");
        second["server_name"] = {"second.com"};
        f.setRoot("root /var/www/second/");
        config::ServerConfig sc;
        sc.addServer(f);
        sc.addServer(second);

        const config::ServerBlock *sb_second = sc.getServer(8080, "second.com");
        const config::ServerBlock *sb_first = sc.getServer(8080, "www.first.com");
        const config::ServerBlock *sb_default = sc.getServer(8080, "notfound.com");

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
