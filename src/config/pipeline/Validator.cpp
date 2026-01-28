#include "config/pipeline/Validator.hpp"
#include "common/filesystem.hpp"
#include "common/string.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/ConfigException.hpp"
#include "utils/Logger.hpp"
#include <map>
#include <set>

namespace config {

Validator::Validator() : perform_fs_checks_(true) {}
Validator::Validator(bool perform_fs_checks) : perform_fs_checks_(perform_fs_checks) {}

void Validator::validate(ServerBlockVec &servers, bool perform_fs_checks) {
    Validator vl(perform_fs_checks);
    if (servers.empty()) {
        throw ConfigError("at least one server is required in config file");
    }
    for (size_t i = 0; i < servers.size(); i++) {
        vl.validateServer(servers[i]);
    }
    vl.validateGlobalConstraints(servers);
}

void Validator::validateServer(ServerBlock &b) {
    // Basic requirement check
    if (b.port() == -1) {
        LOG_WARN("in server block listen is not specified default address: '"
                 << b.address() << "' and port: '9191' is used.");
        b.port(9191);
    }
    if (b.address().empty()) {
        b.address("0.0.0.0");
    }
}

void Validator::validateGlobalConstraints(ServerBlockVec const &servers) {
    std::map<std::string, std::set<std::string> > listenMap;

    for (size_t i = 0; i < servers.size(); ++i) {
        std::string listenKey = servers[i].address() + ":" + utils::toString(servers[i].port());
        std::vector<std::string> names;
        if (servers[i].has("server_name")) {
            names = servers[i].getRawValues("server_name");
        }

        if (names.empty()) {
            names.push_back(""); // Default "unnamed" server
        }

        for (size_t j = 0; j < names.size(); ++j) {
            if (!listenMap[listenKey].insert(names[j]).second) {
                if (names[j].empty()) {
                    LOG_WARN("Conflicting default/unnamed server for " << listenKey);
                } else {
                    LOG_WARN("Conflicting server name '" << names[j] << "' for " << listenKey);
                }
            }
        }
    }
}

} // namespace config
