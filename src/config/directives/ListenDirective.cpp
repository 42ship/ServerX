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
    ServerBlock *serverBlock = dynamic_cast<ServerBlock *>(&b);
    if (!serverBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.name());
    }

    EXPECT_ARG_COUNT(args, 1, name_);

    utils::IpInfo ip_info;
    if (!utils::extractIpInfo(args[0].literal, ip_info)) {
        throw ConfigError("'listen' directive '" + args[0].literal + "' has an invalid format.");
    }

    serverBlock->port((ip_info.port == -1 ? 9191 : ip_info.port));
    serverBlock->address((ip_info.ip.empty() ? "0.0.0.0" : ip_info.ip));
    b.add(name_, args);
}

} // namespace config
