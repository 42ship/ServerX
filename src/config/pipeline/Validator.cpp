#include "config/pipeline/Validator.hpp"
#include "common/filesystem.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/ConfigException.hpp"
#include "utils/Logger.hpp"

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
}

void Validator::validateServer(ServerBlock &b) const {
    validateListen(b);
    for (LocationBlockMap::iterator it = b.locations_.begin(); it != b.locations_.end(); ++it) {
        validateLocation(it->second, b);
    }
}

void Validator::validateLocation(LocationBlock &b, ServerBlock const &server) const {
    (void)server;
    validateRoot(b);
}

void Validator::validateRoot(Block &b) const {
    if (!b.has("root"))
        return;
    std::string const &root = b.root();
    if (perform_fs_checks_) {
        char const *error = utils::validateDirectoryPath(root.c_str());
        if (error) {
            LOG_WARN("'" << root << "': " << error);
            return;
        }
    }
    if (root[root.length() - 1] != '/')
        b.root(root + '/');
}

void Validator::validateListen(ServerBlock const &b) {
    if (!b.has("listen")) {
        LOG_WARN("in server block listen is not specified default address: '"
                 << b.address() << "' and port: '" << b.port() << "' is used.");
        return;
    }
    return;
}

void Validator::validateServerNames(ServerBlock const &b) {
    if (!b.has("server_names"))
        return;
}

} // namespace config
