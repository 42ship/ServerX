
#include "config/pipeline/Parser.hpp"
#include "config/ConfigException.hpp"
#include "config/pipeline/Lexer.hpp"
#include "doctest.h"

TEST_CASE("Parser") {
    SUBCASE("Should parse a simple valid config") {
        const std::string SIMPLE_CONFIG = "server {\n"
                                          "    listen 8080;\n"
                                          "    server_name www.example.com;\n"
                                          "    location / {\n"
                                          "        root /var/www/html;\n"
                                          "        index index.html index.htm;\n"
                                          "    }\n"
                                          "}\n";
        config::TokenArray tokens = config::Lexer::tokenize(SIMPLE_CONFIG);
        config::ConfigNodeVec nodes = config::Parser::parse(tokens);

        CHECK(nodes.size() == 1);
        config::ConfigNode const &server = nodes[0];
        CHECK(server.name == "server");
        CHECK(server.directives.size() == 2);
        CHECK(server.directives.at("listen").size() == 1);
        CHECK(server.directives.at("listen")[0] == "8080");
        CHECK(server.children.size() == 1);

        config::ConfigNode const &loc = server.children[0];
        CHECK(loc.name == "location");
        CHECK(loc.args.size() == 1);
        CHECK(loc.args[0] == "/");
        CHECK(loc.directives.size() == 2);
        CHECK(loc.directives.at("index").size() == 2);
        CHECK(loc.directives.at("index")[1] == "index.htm");
    }

    SUBCASE("Should parse multiple server blocks") {
        const std::string MULTI_SERVER = "server { listen 80; } server { listen 443; }";
        config::TokenArray tokens = config::Lexer::tokenize(MULTI_SERVER);
        config::ConfigNodeVec nodes = config::Parser::parse(tokens);
        CHECK(nodes.size() == 2);
        CHECK(nodes[0].directives.at("listen")[0] == "80");
        CHECK(nodes[1].directives.at("listen")[0] == "443");
    }

    SUBCASE("Should throw on missing semicolon") {
        const std::string MISSING_SEMICOLON = "server { listen 8080 }";
        config::TokenArray tokens = config::Lexer::tokenize(MISSING_SEMICOLON);
        CHECK_THROWS_AS(config::Parser::parse(tokens), const config::ConfigError &);
    }

    SUBCASE("Should throw on missing opening brace") {
        const std::string MISSING_BRACE = "server listen 8080; }";
        config::TokenArray tokens = config::Lexer::tokenize(MISSING_BRACE);
        CHECK_THROWS_AS(config::Parser::parse(tokens), const config::ConfigError &);
    }

    SUBCASE("Should throw on missing closing brace") {
        const std::string MISSING_BRACE = "server { listen 8080;";
        config::TokenArray tokens = config::Lexer::tokenize(MISSING_BRACE);
        CHECK_THROWS_AS(config::Parser::parse(tokens), const config::ConfigError &);
    }

    SUBCASE("Should throw on unexpected token") {
        const std::string UNEXPECTED_TOKEN = "server { ; }";
        config::TokenArray tokens = config::Lexer::tokenize(UNEXPECTED_TOKEN);
        CHECK_THROWS_AS(config::Parser::parse(tokens), const config::ConfigError &);
    }
}
