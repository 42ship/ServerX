#include "config/ServerConfig.hpp"

#include <fstream>
#include <sstream>
#include <iostream>
#include "config/pipeline/Lexer.hpp"
#include "config/pipeline/Parser.hpp"
#include "config/pipeline/Mapper.hpp"
#include "config/pipeline/Validator.hpp"
#include "utils/Logger.hpp"

namespace config {

ServerConfig::ServerConfig(std::string const &content) {
    TokenArray tokens = Lexer::tokenize(content);
    std::vector<ConfigNode> ir = Parser::parse(tokens);
    servers_ = Mapper::map(ir);
    Validator::validate(servers_);
    LOG_TRACE(*this);
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
    servers_ = Mapper::map(ir);
    Validator::validate(servers_);
    LOG_TRACE(*this);
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

std::ostream &operator<<(std::ostream &o, const ServerConfig &t) {
    o << "\n######################################\n";
    o << "#    Server Configuration Summary    #\n";
    o << "######################################\n\n";

    const ServerBlockVec &servers = t.getServers();
    o << "Found " << servers.size() << " server block(s).\n\n";

    for (ServerBlockVec::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        o << *it;
        if (it + 1 != servers.end()) {
            o << "\n--------------------------------------\n\n";
        }
    }

    o << "\n### End of Configuration ###";
    return o;
}
} // namespace config
