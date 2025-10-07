#include "config/internal/Token.hpp"

#include <iostream>

namespace config {

static char const *getTokenTypeStr(TokenType t) {
    // clang-format off
    switch (t) {
    case IDENTIFIER: return "identifier";
    case STRING: return "string";
    case NUMBER: return "number";
    case ENDPOINT: return "endpoint";
    case LEFT_BRACE: return "left_brace";
    case RIGHT_BRACE: return "right_brace";
    case SEMICOLON: return "semicolon";
    case END_OF_FILE: return "end_of_file";
    default:
        return "unknown";
    }
    // clang-format on
}

std::ostream &operator<<(std::ostream &o, TokenType const t) {
    o << getTokenTypeStr(t);
    return o;
}

std::ostream &operator<<(std::ostream &o, Token const &t) {
    o << "T (" << t.literal << "), (" << t.type << ");";
    return o;
}

bool Token::isTypeIn(TokenType const arr[], size_t size) const {
    if (!arr)
        return false;
    for (size_t i = 0; i < size; i++) {
        if (arr[i] == type)
            return true;
    }
    return false;
}

bool Token::operator<(Token const &rhs) const { return literal < rhs.literal; }
bool Token::operator>(Token const &rhs) const { return literal > rhs.literal; }

} // namespace config
