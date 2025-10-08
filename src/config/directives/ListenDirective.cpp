#include "config/directives/ListenDirective.hpp"
#include "config/ConfigException.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/Block.hpp"
#include "config/internal/utils.hpp"
#include <string>

namespace config {

const std::string ListenDirective::name_ = "listen";

void ListenDirective::process(Block &b, StringVector const &args) const {
    ServerBlock *serverBlock = dynamic_cast<ServerBlock *>(&b);
    if (!serverBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.getName());
    }

    if (args.size() != 1) {
        throw ConfigError("'listen' directive requires exactly one argument.");
    }

    utils::IpInfo ip_info;
    if (!utils::extractIpInfo(args[0], ip_info)) {
        throw ConfigError("'listen' directive '" + args[0] + "' has an invalid format.");
    }

    serverBlock->setPort((ip_info.port == -1 ? 9191 : ip_info.port));
    serverBlock->setAddress((ip_info.ip.empty() ? "0.0.0.0" : ip_info.ip));
    b[name_] = args;
}

} // namespace config
