#include "config/directives/ListenDirective.hpp"
#include "config/ConfigException.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/Block.hpp"
#include <string>

namespace config {

const std::string ListenDirective::name_ = "listen";

void ListenDirective::process(Block &b, StringVector const &args) const {
    ServerBlock *serverBlock = dynamic_cast<ServerBlock *>(&b);
    if (!serverBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.getName());
    }

    if (args.size() != 1) {
        throw ConfigException("'listen' directive requires exactly one argument.");
    }

    if (args.size() != 1) {
        throw ConfigError("'listen' directive requires exactly one argument.");
    }
    serverBlock->setListen(args[0]);
    b[name_] = args;
}

} // namespace config
