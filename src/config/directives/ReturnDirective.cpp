#include "config/directives/ReturnDirective.hpp"
#include "config/ConfigException.hpp"
#include "config/arguments/IArgument.hpp"
#include "config/internal/Block.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <string>

namespace config {

const std::string ReturnDirective::name_ = "return";

void ReturnDirective::process(Block &b, ArgumentVector const &args) const {
    (void)b;
    if (args.size() > 2 || args.size() < 1)
        throw ConfigError(
            "see docs for 'return' directive as wrong size of arguments was provided");
    if (args.size() == 2) {
        EXPECT_ARG_TYPE(args[0], ARG_INTEGER, name_);
        EXPECT_ARG_TYPE(args[1], ARG_STRING, name_)
    }
}

} // namespace config
