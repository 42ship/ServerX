#include "config/directives/AutoIndexDirective.hpp"

#include "config/LocationBlock.hpp"
#include "config/arguments/Bool.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string AutoIndexDirective::name_ = "autoindex";

void AutoIndexDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    LocationBlock *locationBlock = dynamic_cast<LocationBlock *>(&b);
    if (!locationBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.name());
    }

    EXPECT_ARG_COUNT(args, 1, name_);

    Bool arg(args[0].literal);
    locationBlock->autoIndex(arg.getValue());
    b.add(name_, new Bool(arg));
}

} // namespace config
