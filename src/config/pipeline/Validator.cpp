#include "config/pipeline/Validator.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "utils/Logger.hpp"

namespace config {

void Validator::validate(ServerBlockVec &servers) {
    if (servers.empty()) {
        throw ConfigError("at least one server is required in config file");
    }
    for (size_t i = 0; i < servers.size(); i++) {
        validateServer(servers[i]);
    }
}

void Validator::validateServer(ServerBlock &b) {
    validateListen(b);
    for (LocationBlockMap::iterator it = b.locations_.begin(); it != b.locations_.end(); ++it) {
        validateLocation(it->second);
    }
}

void Validator::validateLocation(LocationBlock &b) {
    validateRoot(b);
}

void Validator::validateRoot(Block &b) {
    if (!b.has("root"))
        return;
}

void Validator::validateListen(ServerBlock &b) {
    if (!b.has("listen")) {
        LOG_WARN("in server block listen is not specified default address: '"
                 << b.address_ << "' and port: '" << b.port_ << "' is used.");
        return;
    }
    return;
}

void Validator::validateServerNames(ServerBlock &b) {
    if (!b.has("server_names"))
        return;
}

} // namespace config
