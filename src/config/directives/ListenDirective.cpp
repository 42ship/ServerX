#include "config/directives/ListenDirective.hpp"
#include "config/ConfigException.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/IArgument.hpp"
#include "config/internal/Block.hpp"
#include "config/internal/ValidationUtils.hpp"
#include "config/internal/utils.hpp"
#include <string>

namespace config {

const std::string ListenDirective::name_ = "listen";

void ListenDirective::process(Block &b, ArgumentVector const &args) const {
    ServerBlock *serverBlock = dynamic_cast<ServerBlock *>(&b);
    if (!serverBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.getName());
    }

    EXPECT_ARG_COUNT(args, 1, name_)

    if (args[0]->getType() != ARG_STRING && args[0]->getType() != ARG_INTEGER) {
        throw ConfigError("'listen' must be a string or integer, not a " +
                          std::string(getArgumentTypeName(args[0]->getType())));
    }

    utils::IpInfo ip_info;
    if (!utils::extractIpInfo(args[0]->getRawValue(), ip_info)) {
        throw ConfigError("'listen' directive '" + args[0]->getRawValue() +
                          "' has an invalid format.");
    }

    serverBlock->setPort((ip_info.port == -1 ? 9191 : ip_info.port));
    serverBlock->setAddress((ip_info.ip.empty() ? "0.0.0.0" : ip_info.ip));
    b[name_] = args;
}

} // namespace config
