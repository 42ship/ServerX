#pragma once
#include "common/string.hpp"
#include "config/Block.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

/**
 * @class ValidatorUtils
 * @brief Static utility class for common configuration validation tasks.
 */
class ValidatorUtils {
public:
    /**
     * @brief Verifies that the number of arguments is within the inclusive range [min, max].
     * @throws ConfigError if the count is outside the range.
     */
    static void checkArgs(ParsedDirectiveArgs const &args, size_t min, size_t max,
                          std::string const &name) {
        if (args.size() < min || args.size() > max) {
            std::string msg = "'" + name + "' directive requires ";
            if (min == max) {
                msg += "exactly " + utils::toString(min);
            } else {
                msg += "between " + utils::toString(min) + " and " + utils::toString(max);
            }
            msg += " argument(s).";
            throw ConfigError(msg);
        }
    }

    /**
     * @brief Verifies that a specific argument matches the expected token type.
     * @throws ConfigError if the type does not match.
     */
    static void checkType(Token const &arg, TokenType expectedType, std::string const &name) {
        if (arg.type != expectedType) {
            throw ConfigError("'" + name + "' expected " + getTokenTypeName(expectedType) + ".");
        }
    }

    /**
     * @brief Verifies if the directive is being processed within an allowed context.
     * @throws ConfigError if the block type is not in the allowed list.
     */
    static void checkContext(Block const &block, std::string const &allowedContexts,
                             std::string const &name) {
        if (allowedContexts.find(block.name()) == std::string::npos) {
            throw ConfigError("'" + name + "' directive is not allowed in: " + block.name());
        }
    }

private:
    static std::string getTokenTypeName(TokenType type) {
        switch (type) {
        case NUMBER:
            return "NUMBER";
        case IDENTIFIER:
            return "IDENTIFIER";
        case STRING:
            return "STRING";
        default:
            return "value";
        }
    }
};

} // namespace config
