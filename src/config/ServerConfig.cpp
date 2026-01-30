#include "config/ServerConfig.hpp"

#include "config/ServerBlock.hpp"
#include "config/pipeline/Lexer.hpp"
#include "config/pipeline/Mapper.hpp"
#include "config/pipeline/Parser.hpp"
#include "config/pipeline/Validator.hpp"
#include "http/Request.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace config {

ServerConfig::ServerConfig() {}
ServerConfig::ServerConfig(std::string const &content, bool perform_fs_checks) {
    build(content, perform_fs_checks);
}

ServerConfig::ServerConfig(char const *fpath, bool perform_fs_checks) {
    std::ifstream file(fpath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file.");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    build(buffer.str(), perform_fs_checks);
}

void ServerConfig::build(std::string const &content, bool perform_fs_checks) {
    TokenArray tokens = Lexer::tokenize(content);
    std::vector<ConfigNode> ir = Parser::parse(tokens);
    ServerBlockVec servers = Mapper::map(ir);
    Validator::validate(servers, perform_fs_checks);
    for (size_t i = 0; i < servers.size(); i++) {
        addServer(servers[i]);
    }
    LOG_TRACE(*this);
}

ServerBlockMap const &ServerConfig::getServersMap() const { return servers_; }

ServerBlock const *ServerConfig::getServer(int port, http::Request const &req) const {
    std::string host = req.headers().get("Host");
    size_t colon_pos = host.find(':');
    if (colon_pos != std::string::npos) {
        host.resize(colon_pos);
    }

    ServerBlockMap::const_iterator it = servers_.find(port);
    if (it == servers_.end())
        return NULL;

    ServerBlockVec const &blocks = it->second;
    for (size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].has("server_name")) {
            std::vector<std::string> names = blocks[i].getRawValues("server_name");
            if (details::matchServerName(names, host)) {
                return &blocks[i];
            }
        }
    }

    return &blocks[0];
}

void ServerConfig::addServer(ServerBlock const &server) {
    servers_[server.port()].push_back(server);
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

    o << "\n### End of Configuration ###";
    return o;
}

} // namespace config
