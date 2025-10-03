#include "config/pipeline/Parser.hpp"

#include "config/ConfigException.hpp"
#include "config/arguments/IArgument.hpp"
#include "config/arguments/Integer.hpp"
#include "config/arguments/String.hpp"
#include "config/internal/ConfigNode.hpp"
#include "config/internal/Token.hpp"
#include "utils/utils.hpp"

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

void Parser::expectToken(std::string literal) {
    if (currentToken().literal != literal)
        throw ConfigError("Non matching token literals");
    consumeToken();
}

void Parser::addDirective(ConfigNode &node, DirectivePair const &pair) const {
    DirectiveMap::iterator it = node.directives.find(pair.first);

    if (it != node.directives.end()) {
        DirectiveArgs &ar = it->second;
        ar.reserve(ar.size() + pair.second.size());
        ar.insert(ar.end(), pair.second.begin(), pair.second.end());
    } else {
        node.directives.insert(pair);
    }
}

bool Parser::isTokenAValue() const {
    static const TokenType validTypes[] = {IDENTIFIER, STRING, NUMBER};
    static const size_t size = sizeof(validTypes) / sizeof(validTypes[0]);
    return currentToken().isTypeIn(validTypes, size);
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
    if (peekToken().type != LEFT_BRACE)
        throw ConfigError("Non matching token types");
    if (!isTokenAValue())
        throw ConfigError("Expected a location name (identifier)");

    ConfigNode node(std::string("location"));
    pushTokenTo(node.args);

    consumeToken();
    expectToken(LEFT_BRACE);
    while (currentToken().type != RIGHT_BRACE)
        addDirective(node, handleDirective());
    expectToken(RIGHT_BRACE);
    nodes_.back().children.push_back(node);
}

DirectivePair Parser::handleDirective() {
    if (!isTokenAValue())
        throw ConfigError("Expected a directive name (identifier).");

    DirectivePair d;
    d.first = currentToken().literal;
    consumeToken();

    while (isTokenAValue()) {
        pushTokenTo(d.second);
        consumeToken();
    }
    expectToken(SEMICOLON);
    return d;
}

void Parser::pushTokenTo(DirectiveArgs &args) {
    IArgument *arg;
    std::string const &s = currentToken().literal;

    // clang-format off
    switch (currentToken().type) {
    case STRING: arg = new String(s); break;
    case NUMBER: arg = new Integer(utils::fromString<int>(s)); break;
    default:
        throw ConfigError("Internal error on pushing token into directive args");
    }
    // clang-format on
    args.push_back(arg);
}

} // namespace config
