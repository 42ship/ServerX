#include "config/directives/AutoIndexDirective.hpp"

#include "config/LocationBlock.hpp"
#include "config/arguments/Bool.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string AutoIndexDirective::name_ = "autoindex";

void AutoIndexDirective::process(Block &block, ParsedDirectiveArgs const &args) const {
    LocationBlock *locationBlock = dynamic_cast<LocationBlock *>(&block);
    if (!locationBlock) {
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + block.name());
    }

    EXPECT_ARG_COUNT(args, 1, name_);

    Bool arg(args[0].literal);
    locationBlock->autoIndex(arg.getValue());
    block.add(name_, new Bool(arg));
}

} // namespace config
