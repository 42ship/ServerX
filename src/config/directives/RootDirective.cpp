#include "config/directives/RootDirective.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <string>

namespace config {

const std::string RootDirective::name_ = "root";

void RootDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 1, 1, name_);

    b.root(args[0].literal);
}

} // namespace config
