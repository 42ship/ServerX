#pragma once

#include <vector>
#include "../internal/Token.hpp"

namespace config {

typedef std::vector<Token> TokenArray;

/**
 * @class Lexer
 * @brief A static utility class that performs lexical analysis on a config string.
 *
 * The Lexer breaks down a raw string into a sequence of Tokens, which can then
 * be consumed by the Parser. It handles basic elements like keywords, strings,
 * numbers, and punctuation, ignoring whitespace and comments.
 */
class Lexer {
public:
    static void tokenize(std::string const &content, TokenArray &res);
    static TokenArray tokenize(std::string const &content);
    static void printTokens(TokenArray const &tokens);

private:
    Lexer(std::string const &content, std::vector<Token> &res);

    std::string const &content_;
    TokenArray &tokens_;
    size_t pos_;
    char ch_;

    void run();

    void pushToken(TokenType type, std::string const &literal);
    void pushToken(TokenType type, char);
    void readChar();
    void eatWhitespaces();
    void identifyLastToken();

    bool isPunctuation();
    bool handleValue();
    void handleComment();
    void handleUnknown();
    void handlePunctuation();
};

} // namespace config
