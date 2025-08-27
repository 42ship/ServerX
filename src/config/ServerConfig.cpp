#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

#include "Lexer.hpp"
#include "Parser.hpp"
#include "ConfigBuilder.hpp"
#include "ServerConfig.hpp"

using namespace config;

ServerConfig::ServerConfig(std::string const &content) {
    TokenArray tokens = Lexer::tokenize(content);
    std::vector<ConfigNode> ir = Parser::parse(tokens);
    servers_ = ConfigBuilder::build(ir);
}

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
    size_t temp = server_name.find(':');
    std::string hostname;
    if (temp != std::string::npos) {
        hostname = server_name.substr(0, temp);
    } else
        hostname = server_name;
    for (size_t i = 0; i < servers_.size(); i++) {
        ServerBlock const &block = servers_[i];
        if (block.port_ == port && block.matchServerName(hostname)) {
            return &block;
        }
    }
    return NULL;
}
