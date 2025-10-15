#include "config/directives/RootDirective.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include <string>

namespace config {

const std::string RootDirective::name_ = "root";

void RootDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.size() != 1)
        throw ConfigError("'root' directive requires exactly one argument.");
    std::string root = args[0].literal;
    if (!dynamic_cast<LocationBlock *>(&b) && !dynamic_cast<ServerBlock *>(&b))
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.name());

    if (!root.empty() && root[root.length() - 1] != '/')
        root += '/';
    b.root(root);
}

} // namespace config
