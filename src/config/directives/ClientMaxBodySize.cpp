#include "config/directives/ClientMaxBodySize.hpp"
#include "common/string.hpp"
#include "config/arguments/Integer.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <string>

namespace config {

const std::string ClientMaxBodySize::name_ = "client_max_body_size";

void ClientMaxBodySize::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 1, 1, name_);

    if (args[0].type != NUMBER && args[0].type != STRING && args[0].type != IDENTIFIER) {
        throw ConfigError("'" + name_ + "' invalid value type.");
    }

    std::string const &literal = args[0].literal;
    if (literal.empty()) {
        throw ConfigError("'" + name_ + "' value cannot be empty.");
    }

    size_t multiplier = 1;
    std::string numberPart = literal;

    char unit = literal[literal.length() - 1];
    if (unit == 'k' || unit == 'K') {
        multiplier = 1024;
        numberPart = literal.substr(0, literal.length() - 1);
    } else if (unit == 'm' || unit == 'M') {
        multiplier = 1024 * 1024;
        numberPart = literal.substr(0, literal.length() - 1);
    } else if (unit == 'g' || unit == 'G') {
        multiplier = 1024 * 1024 * 1024;
        numberPart = literal.substr(0, literal.length() - 1);
    }

    if (numberPart.empty() || !utils::isAllDigit(numberPart)) {
        throw ConfigError("'" + name_ + "' invalid numeric value: " + literal);
    }

    size_t size = utils::fromString<size_t>(numberPart);

    if (multiplier > 0 && size > (static_cast<size_t>(-1) / multiplier)) {
        throw ConfigError("'" + name_ + "' value is too large (overflow): " + literal);
    }

    size_t total = size * multiplier;
    b.add(name_, new Integer(total));
}

} // namespace config
