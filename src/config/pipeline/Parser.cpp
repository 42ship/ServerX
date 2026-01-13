#include "config/pipeline/Parser.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/ConfigNode.hpp"
#include "config/internal/Token.hpp"

namespace config {

Parser::Parser(TokenArray const &tokens) : tokens_(tokens), pos_(0) { nodes_.reserve(2); }

std::vector<ConfigNode> Parser::parse(TokenArray const &tokens) {
    Parser parser(tokens);
    parser.handleServerBlock();
    return parser.nodes_;
}

size_t Parser::size() const { return tokens_.size(); }

Token const &Parser::currentToken() const {
    if (pos_ >= size())
        return tokens_.back();
    return tokens_[pos_];
}

Token const &Parser::peekToken() const {
    if (pos_ + 1 >= size())
        return tokens_.back();
    return tokens_[pos_ + 1];
}

void Parser::consumeToken() {
    if (pos_ < size())
        pos_++;
}

void Parser::expectToken(TokenType type) {
    if (currentToken().type != type)
        throw ConfigError("Non matching token types");
    consumeToken();
}

void Parser::expectToken(std::string const &literal) {
    if (currentToken().literal != literal)
        throw ConfigError("Non matching token literals");
    consumeToken();
}

void Parser::addDirective(ConfigNode &node, ParsedDirectivePair const &pair) {
    ParsedDirectiveMap::iterator it = node.directives.find(pair.first);

    if (it != node.directives.end()) {
        ParsedDirectiveArgs &ar = it->second;
        ar.reserve(ar.size() + pair.second.size());
        ar.insert(ar.end(), pair.second.begin(), pair.second.end());
    } else {
        node.directives.insert(pair);
    }
}

bool Parser::isTokenAValue() const {
    static const TokenType validTypes[] = {IDENTIFIER, STRING, NUMBER};
    static const size_t validTypesCount = sizeof(validTypes) / sizeof(validTypes[0]);
    return currentToken().isTypeIn(validTypes, validTypesCount);
}

void Parser::handleServerBlock() {
    while (currentToken().type != END_OF_FILE) {
        expectToken("server");
        expectToken(LEFT_BRACE);
        nodes_.push_back(ConfigNode("server"));
        while (currentToken().type != RIGHT_BRACE)
            handleStatement();
        expectToken(RIGHT_BRACE);
    }
}

void Parser::handleStatement() {
    if (currentToken().literal == "location")
        handleLocationBlock();
    else
        addDirective(nodes_.back(), handleDirective());
}

void Parser::handleLocationBlock() {
    expectToken("location");

    ConfigNode node(std::string("location"));
    while (currentToken().type != LEFT_BRACE && currentToken().type != END_OF_FILE) {
        if (!isTokenAValue())
            throw ConfigError("Expected a location name (identifier)");
        pushTokenTo(node.args);
        consumeToken();
    }

    if (node.args.empty())
        throw ConfigError("Expected a location name (identifier)");

    expectToken(LEFT_BRACE);
    while (currentToken().type != RIGHT_BRACE)
        addDirective(node, handleDirective());
    expectToken(RIGHT_BRACE);
    nodes_.back().children.push_back(node);
}

ParsedDirectivePair Parser::handleDirective() {
    if (!isTokenAValue())
        throw ConfigError("Expected a directive name (identifier).");

    ParsedDirectivePair d;
    d.first = currentToken().literal;
    consumeToken();

    while (isTokenAValue()) {
        pushTokenTo(d.second);
        consumeToken();
    }
    expectToken(SEMICOLON);
    return d;
}

void Parser::pushTokenTo(ParsedDirectiveArgs &args) const { args.push_back(currentToken()); }

} // namespace config
