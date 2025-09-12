#pragma once

#include <string>
#include <ostream>

namespace config {

/**
 * @enum TokenType
 * @brief Represents the type of a lexical token.
 */
typedef enum TokenType {
    IDENTIFIER,  //!< A keyword or variable name, e.g., "server_name"
    STRING,      //!< A quoted string literal, e.g., "hello world"
    NUMBER,      //!< A numeric literal, e.g., 8080
    ENDPOINT,    //!< A special token type, perhaps for future use
    LEFT_BRACE,  //!< The '{' character
    RIGHT_BRACE, //!< The '}' character
    SEMICOLON,   //!< The ';' character
    UNKNOWN,     //!< A token that could not be identified
    END_OF_FILE  //!< A special token representing the end of the input
} TokenType;

/**
 * @struct Token
 * @brief Represents a single lexical token with a type and a literal value.
 */
typedef struct Token {
    std::string literal;
    TokenType type;

    /**
     * @brief Checks if the token's type is present in a given array of types.
     * @param arr An array of TokenType to check against.
     * @param size The size of the array.
     * @return True if the token's type is in the array, false otherwise.
     */
    bool isTypeIn(TokenType const arr[], size_t size) const;
} Token;

std::ostream &operator<<(std::ostream &o, Token const &t);
std::ostream &operator<<(std::ostream &o, TokenType const t);

} // namespace config
