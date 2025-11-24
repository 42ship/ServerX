#pragma once

#include "../internal/ConfigNode.hpp"
#include "Lexer.hpp"

/* GRAMMAR in EBNF
 * 	config_file = { server_block } ;
 *	server_block = "server" "{" { statement } "}" ;
 *	statement = directive | location_block ;
 *	directive = IDENTIFIER { parameter } ";" ;
 *	location_block = "location" IDENTIFIER "{" { directive } "}" ;
 *	parameter = IDENTIFIER | STRING | NUMBER ;
 */

namespace config {

/**
 * @class Parser
 * @brief Performs syntax analysis on a stream of tokens to build a configuration tree.
 *
 * The Parser consumes tokens from the Lexer and validates them against a defined
 * grammar. Its primary role is to build a tree of ConfigNode objects (the IR)
 * that represents the structure of the configuration file. It ensures syntactic
 * correctness but does not validate the meaning of directives.
 *
 * @note This class is responsible for reporting **syntax errors** (e.g., missing ';').
 * Semantic errors (e.g., invalid port number) are handled by the ConfigBuilder.
 */
class Parser {
public:
    static std::vector<ConfigNode> parse(TokenArray const &);

private:
    std::vector<ConfigNode> nodes_;
    TokenArray const &tokens_;
    size_t pos_;

    Parser();
    explicit Parser(TokenArray const &);
    size_t size() const;

    Token const &currentToken() const;
    Token const &peekToken() const;
    void consumeToken();
    void expectToken(TokenType type);
    void expectToken(std::string const &literal);
    bool isTokenAValue() const;
    void displayCurrentToken() const;
    static void addDirective(ConfigNode &node, ParsedDirectivePair const &pair);
    void pushTokenTo(ParsedDirectiveArgs &args) const;

    void handleServerBlock();
    void handleStatement();
    void handleLocationBlock();
    ParsedDirectivePair handleDirective();
};

} // namespace config
