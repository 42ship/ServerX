#include "config/directives/ReturnDirective.hpp"
#include "common/string.hpp"
#include "config/Block.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

const std::string ReturnDirective::name_ = "return";

void ReturnDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.empty())
        throw ConfigError("'" + name_ + "' requires at least one argument");
    if (args.size() > 2)
        throw ConfigError("'" + name_ + "' too many arguments");
    if (args.size() == 1) {
        b.add(name_, args);
        return;
    }
    if (args[0].type != NUMBER) {
        throw ConfigError("'" + name_ + "' expected INTEGER type.");
    }
    int code = utils::fromString<int>(args[0].literal);
    if (code < 100 || code >= 1000) {
        throw ConfigError("in '" + name_ + "' directive Http Status code is malformed.");
    }
    b.add(name_, args);
}

} // namespace config
