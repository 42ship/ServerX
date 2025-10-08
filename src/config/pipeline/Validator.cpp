#include "config/pipeline/Validator.hpp"
#include "common/filesystem.hpp"
#include "config/ConfigException.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
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

void Validator::validateServer(ServerBlock &b) {
    validateListen(b);
    for (LocationBlockMap::iterator it = b.locations().begin(); it != b.locations().end(); ++it) {
        validateLocation(it->second, b);
    }
}

void Validator::validateLocation(LocationBlock &b, ServerBlock const &server) {
    locationCompleteRoot(b, server);
    validateRoot(b);
}

void Validator::validateRoot(Block &b) {
    if (!b.has("root"))
        return;
    std::string const &root = b.getRoot();
    if (perform_fs_checks_) {
        char const *error = utils::validateDirectoryPath(root.c_str());
        if (error) {
            LOG_WARN("'" << root << "': " << error);
            return;
        }
    }
    if (root[root.length() - 1] != '/')
        b.setRoot(root + '/');
}

void Validator::validateListen(ServerBlock &b) {
    if (!b.has("listen")) {
        LOG_WARN("in server block listen is not specified default address: '"
                 << b.getAddress() << "' and port: '" << b.getPort() << "' is used.");
        return;
    }
    return;
}

void Validator::validateServerNames(ServerBlock &b) {
    if (!b.has("server_names"))
        return;
}

// ==============================HELPER FUNCTIONS==============================

void Validator::locationCompleteRoot(LocationBlock &l, ServerBlock const &s) {
    std::string finalRoot;

    std::string serverRoot = s.getRoot();
    std::string locationRoot = l.getRoot();
    if (!locationRoot.empty()) {
        // root is an absolute path
        if (locationRoot[0] == '/') {
            finalRoot = locationRoot;
        } else { // root is a relative path
            finalRoot = serverRoot;
            if (!finalRoot.empty() && finalRoot[finalRoot.length() - 1] != '/') {
                finalRoot += '/';
            }
            finalRoot += locationRoot;
        }
    } else {
        finalRoot = serverRoot;
    }
    if (finalRoot.empty()) {
        issue_warning("Could not determine a root path for location '" + l.getPath() + "'.");
        return;
    }
    l.setRoot(finalRoot);
}

} // namespace config
