#include "config/directives/ListenDirective.hpp"
#include "config/Block.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "config/internal/ValidationUtils.hpp"
#include "config/internal/types.hpp"
#include "config/internal/utils.hpp"
#include <string>

namespace config {

const std::string ListenDirective::name_ = "listen";

void ListenDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server", name_);
    ValidatorUtils::checkArgs(args, 1, 1, name_);

    utils::IpInfo ipInfo;
    if (!utils::extractIpInfo(args[0].literal, ipInfo)) {
        throw ConfigError("'listen' directive '" + args[0].literal + "' has an invalid format.");
    }

    ServerBlock *serverBlock = static_cast<ServerBlock *>(&b);
    serverBlock->port((ipInfo.port == -1 ? 9191 : ipInfo.port));
    serverBlock->address((ipInfo.ip.empty() ? "0.0.0.0" : ipInfo.ip));
    b.add(name_, args);
}

} // namespace config
