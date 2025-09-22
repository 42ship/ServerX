#include "config/ServerConfig.hpp"

#include "config/ServerBlock.hpp"
#include "config/pipeline/Lexer.hpp"
#include "config/pipeline/Mapper.hpp"
#include "config/pipeline/Parser.hpp"
#include "config/pipeline/Validator.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace config {

ServerConfig::ServerConfig(std::string const &content) {
    build(content);
}

ServerConfig::ServerConfig(char const *fpath) {
    std::ifstream file(fpath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    build(buffer.str());
}

void ServerConfig::build(std::string const &content) {
    TokenArray tokens = Lexer::tokenize(content);
    std::vector<ConfigNode> ir = Parser::parse(tokens);
    ServerBlockVec servers = Mapper::map(ir);
    Validator::validate(servers);
    for (size_t i = 0; i < servers.size(); i++) {
        servers_[servers[i].getPort()].push_back(servers[i]);
    }
    LOG_TRACE(*this);
}

ServerBlockMap const &ServerConfig::getServersMap() const {
    return servers_;
}

ServerBlock const *ServerConfig::getServer(int port, std::string const &server_name) const {
    if (server_name.empty())
        return NULL;
    ServerBlockMap::const_iterator it = servers_.find(port);
    if (it == servers_.end())
        return NULL;
    return &it->second[0];
}

std::ostream &operator<<(std::ostream &o, const ServerConfig &t) {
    o << "\n######################################\n";
    o << "#    Server Configuration Summary    #\n";
    o << "######################################";

    ServerBlockMap const &server_map = t.getServersMap();
    if (server_map.empty()) {
        o << "\n\nNo server blocks are configured.\n";
    }

    for (ServerBlockMap::const_iterator port_entry = server_map.begin();
         port_entry != server_map.end(); ++port_entry) {

        o << "\n\nListening on Port " << port_entry->first << " (" << port_entry->second.size()
          << " server block(s))";
        o << "\n======================================";

        ServerBlockVec const &blocks = port_entry->second;
        for (ServerBlockVec::const_iterator it = blocks.begin(); it != blocks.end(); ++it) {

            if (it != blocks.begin()) {
                o << "\n--------------------------------------";
            }
            o << "\n" << *it;
        }
    }

    o << "\n\n### End of Configuration ###\n";
    return o;
}

} // namespace config
