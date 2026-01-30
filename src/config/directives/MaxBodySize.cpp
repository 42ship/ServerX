#include "config/directives/MaxBodySize.hpp"
#include "common/string.hpp"
#include "config/arguments/Integer.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include <string>

namespace config {

const std::string MaxBodySize::name_ = "max_body_size";

void MaxBodySize::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.size() != 1)
        throw ConfigError("'" + name_ + "' directive requires exactly one argument.");
    size_t size = utils::fromString<unsigned int>(args[0].literal) * 1024 * 1024;
    b.add(name_, new Integer(size));
}

} // namespace config
