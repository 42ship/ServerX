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

    std::cout << "\n=== DEBUG: Intermediate Representation ===" << std::endl;
    std::cout << "Number of server blocks: " << ir.size() << std::endl;

    for (size_t i = 0; i < ir.size(); i++) {
        std::cout << "\nServer Block " << i << ":" << std::endl;
        std::cout << "  Name: " << ir[i].name << std::endl;

        // Print arguments if any
        if (!ir[i].args.empty()) {
            std::cout << "  Args:";
            for (size_t j = 0; j < ir[i].args.size(); j++) {
                std::cout << " " << ir[i].args[j];
            }
            std::cout << std::endl;
        }

        // Print directives
        std::cout << "  Directives:" << std::endl;
        for (DirectiveMap::const_iterator it = ir[i].directives.begin();
             it != ir[i].directives.end(); ++it) {
            std::cout << "    " << it->first << ":";
            for (size_t j = 0; j < it->second.size(); j++) {
                std::cout << " " << it->second[j];
            }
            std::cout << std::endl;
        }

        // Print children (location blocks)
        if (!ir[i].children.empty()) {
            std::cout << "  Locations:" << std::endl;
            for (size_t j = 0; j < ir[i].children.size(); j++) {
                const ConfigNode &loc = ir[i].children[j];
                std::cout << "    Location: " << loc.name;
                if (!loc.args.empty()) {
                    std::cout << " " << loc.args[0];
                }
                std::cout << std::endl;

                // Print location directives
                for (DirectiveMap::const_iterator it = loc.directives.begin();
                     it != loc.directives.end(); ++it) {
                    std::cout << "      " << it->first << ":";
                    for (size_t k = 0; k < it->second.size(); k++) {
                        std::cout << " " << it->second[k];
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    std::cout << "=== END DEBUG ===" << std::endl << std::endl;

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
