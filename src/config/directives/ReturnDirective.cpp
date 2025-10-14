#include "config/directives/ReturnDirective.hpp"
#include "config/internal/ConfigException.hpp"
#include "utils/utils.hpp"
#include <string>

namespace config {

const std::string ReturnDirective::name_ = "return";

void ReturnDirective::process(Block &b, StringVector const &args) const {
    (void)b;
    if (args.size() > 2 || args.size() < 1)
        throw ConfigError(
            "see docs for 'return' directive as wrong size of arguments was provided");
    if (args.size() == 2) {
        if (args[0].empty())
            throw ConfigError("the status code for return directive is empty");
        if (!utils::isAllDigit(args[0]))
            throw ConfigError("the status code must contain only digits");
    }
}

} // namespace config
