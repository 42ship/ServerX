#include "Lexer.hpp"
#include <iostream>

namespace config {

Lexer::Lexer(std::string const &content, TokenArray &res)
    : content_(content), tokens_(res), pos_(0), ch_(0) {
    readChar();
}

void Lexer::tokenize(std::string const &content, TokenArray &res) {
    Lexer lex(content, res);
    lex.run();
}

TokenArray Lexer::tokenize(std::string const &content) {
    TokenArray arr;
    Lexer lex(content, arr);
    lex.run();
    return arr;
}

void Lexer::run() {
    while (ch_) {
        eatWhitespaces();
        if (!ch_)
            break;
        if (isPunctuation()) {
            handlePunctuation();
            continue;
        }
        if (handleValue())
            identifyLastToken();
        else
            handleUnknown();
    }
    pushToken(END_OF_FILE, "");
}

void Lexer::handleComment() {
    while (ch_ != '\n' && ch_)
        readChar();
}

void Lexer::handleUnknown() {
    pushToken(UNKNOWN, ch_);
}

void Lexer::handlePunctuation() {
    if (ch_ == '#')
        return (handleComment());
    else if (ch_ == '{')
        pushToken(LEFT_BRACE, ch_);
    else if (ch_ == '}')
        pushToken(RIGHT_BRACE, ch_);
    else if (ch_ == ';')
        pushToken(SEMICOLON, ch_);
    else
        return;
    readChar();
}

bool Lexer::isPunctuation() {
    return ch_ == '#' || ch_ == '{' || ch_ == '}' || ch_ == ';';
}

bool Lexer::handleValue() {
    size_t start_pos = pos_ - 1;
    while (ch_ && !isPunctuation() && !std::isspace(ch_)) {
        readChar();
    }
    if (start_pos + 1 == pos_)
        return false;
    pushToken(STRING, content_.substr(start_pos, (pos_ - 1) - start_pos));
    return true;
}

void Lexer::readChar() {
    ch_ = (pos_ < content_.length()) ? content_[pos_] : 0;
    pos_++;
}

void Lexer::eatWhitespaces() {
    while (std::isspace(ch_))
        readChar();
}

void Lexer::identifyLastToken() {
    bool is_number = 1;
    std::string &literal = tokens_.back().literal;
    for (size_t i = 0; i < literal.length(); i++) {
        if (!::isdigit(literal[i])) {
            is_number = 0;
            break;
        }
    }
    if (is_number)
        tokens_.back().type = NUMBER;
}

void Lexer::pushToken(TokenType type, char c) {
    Token tok;
    tok.literal += c;
    tok.type = type;
    tokens_.push_back(tok);
}

void Lexer::pushToken(TokenType type, std::string const &literal) {
    Token tok;
    tok.literal = literal;
    tok.type = type;
    tokens_.push_back(tok);
}

void Lexer::printTokens(TokenArray const &tokens) {
    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout << tokens[i].literal << " -> ";
    }
    std::cout << std::endl;
}

} // namespace config