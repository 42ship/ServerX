#include "config/directives/RootDirective.hpp"
#include "config/ConfigException.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/Block.hpp"
#include "config/internal/Token.hpp"
#include "config/internal/ValidationUtils.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

const std::string RootDirective::name_ = "root";

void RootDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (!dynamic_cast<LocationBlock *>(&b) && !dynamic_cast<ServerBlock *>(&b))
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.getName());
    EXPECT_ARG_COUNT(args, 1, name_)
    // EXPECT_ARG_TYPE(args[0], ARG_STRING, name_)
    std::string root = args[0].literal;
    if (!root.empty() && root[root.length() - 1] != '/')
        root += '/';
    b.setRoot(root);
}

} // namespace config
