#include "config/pipeline/Lexer.hpp"
#include "doctest.h"

TEST_CASE("Lexer") {
    SUBCASE("Should tokenize a simple config correctly") {
        const std::string SIMPLE_CONFIG = "server {\n"
                                          "    listen 8080;\n"
                                          "    server_name www.example.com;\n"
                                          "    location / {\n"
                                          "        root /var/www/html;\n"
                                          "        index index.html index.htm;\n"
                                          "    }\n"
                                          "}\n";
        config::TokenArray tokens = config::Lexer::tokenize(SIMPLE_CONFIG);

        // server { listen 8080 ; server_name www.example.com ; location / { root /var/www/html ;
        // index index.html index.htm ; } } EOF
        CHECK(tokens.size() == 21);
        CHECK(tokens[0].literal == "server");
        CHECK(tokens[1].type == config::LEFT_BRACE);
        CHECK(tokens[2].literal == "listen");
        CHECK(tokens[3].literal == "8080");
        CHECK(tokens[3].type == config::NUMBER);
        CHECK(tokens[4].type == config::SEMICOLON);
        CHECK(tokens[20].type == config::END_OF_FILE);
    }

    SUBCASE("Should handle comments") {
        const std::string CONFIG_WITH_COMMENTS = "server { # This is a server block\n"
                                                 "    listen 8080; # Listen on port 8080\n"
                                                 "}";
        config::TokenArray tokens = config::Lexer::tokenize(CONFIG_WITH_COMMENTS);
        // server { listen 8080 ; } EOF
        CHECK(tokens.size() == 7);
        CHECK(tokens[0].literal == "server");
        CHECK(tokens[1].type == config::LEFT_BRACE);
        CHECK(tokens[2].literal == "listen");
        CHECK(tokens[3].literal == "8080");
        CHECK(tokens[4].type == config::SEMICOLON);
        CHECK(tokens[5].type == config::RIGHT_BRACE);
    }

    SUBCASE("Should handle various whitespaces") {
        const std::string CONFIG_WITH_SPACES = "server\t{\n  listen   8080;\r\n}";
        config::TokenArray tokens = config::Lexer::tokenize(CONFIG_WITH_SPACES);
        CHECK(tokens.size() == 7);
        CHECK(tokens[0].literal == "server");
        CHECK(tokens[1].type == config::LEFT_BRACE);
        CHECK(tokens[2].literal == "listen");
        CHECK(tokens[3].literal == "8080");
        CHECK(tokens[4].type == config::SEMICOLON);
        CHECK(tokens[5].type == config::RIGHT_BRACE);
    }

    SUBCASE("Should handle empty input") {
        config::TokenArray tokens = config::Lexer::tokenize("");
        CHECK(tokens.size() == 1);
        CHECK(tokens[0].type == config::END_OF_FILE);
    }
}
