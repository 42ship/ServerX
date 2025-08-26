#include "config/ServerConfig.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ConfigBuilder.hpp"

using namespace config;

ServerConfig::ServerConfig(char const *fpath) {
    std::ifstream file(fpath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    TokenArray tokens = Lexer::tokenize(content);
    std::vector<ConfigNode> ir = Parser::parse(tokens);
    servers_ = ConfigBuilder::build(ir);
}

bool ServerConfig::getServer(int port, std::string const &server_name,
                             ServerBlock const *&res) const {
    if (server_name.empty())
        return false;
    for (size_t i = 0; i < servers_.size(); i++) {
        ServerBlock const &block = servers_[i];
        if (block.port_ == port && block.matchServerName(server_name)) {
            res = &block;
            return true;
        }
    }
    return false;
}

ServerBlock const *ServerConfig::getServer(int port, std::string const &server_name) const {
    if (server_name.empty())
        return NULL;
    for (size_t i = 0; i < servers_.size(); i++) {
        ServerBlock const &block = servers_[i];
        if (block.port_ == port && block.matchServerName(server_name)) {
            return &block;
        }
    }
    return NULL;
}
