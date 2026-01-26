#include "config/directives/IndexDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string IndexDirective::name_ = "index";

void IndexDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 1, 10, name_);

    b.add(name_, args);
}

} // namespace config
