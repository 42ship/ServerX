#include "config/ServerConfig.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include "config/Lexer.hpp"
#include "config/Parser.hpp"
#include "config/ConfigBuilder.hpp"

namespace config {

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
    std::cout << *this << std::endl;
}

ServerBlockVec const &ServerConfig::getServers() const {
    return servers_;
}

ServerBlock const *ServerConfig::getServer(int port, std::string const &server_name) const {
    if (server_name.empty())
        return NULL;
    for (size_t i = 0; i < servers_.size(); i++) {
        ServerBlock const &block = servers_[i];
        if (block.port_ == port) {
            return &block;
        }
    }
    return NULL;
}

std::ostream &operator<<(std::ostream &o, ServerConfig const &t) {
    o << "### Server Configuration Start ###\n\n";
    for (ServerBlockVec::const_iterator it = t.servers_.begin(); it != t.servers_.end(); ++it) {
        o << *it << "\n";
    }
    o << "### Server Configuration End ###\n";
    return o;
}

} // namespace config
