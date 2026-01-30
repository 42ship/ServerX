#include "config/directives/AliasDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string AliasDirective::name_ = "alias";

void AliasDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "location", name_); // Alias is only allowed in location
    ValidatorUtils::checkArgs(args, 1, 1, name_);

    b.add(name_, args);
}

} // namespace config
