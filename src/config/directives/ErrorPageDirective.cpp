#include "config/directives/ErrorPageDirective.hpp"

#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/String.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include <string>

namespace config {
const std::string ErrorPageDirective::name_ = "error_page";

void ErrorPageDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.size() < 2) {
        throw ConfigException("'" + name_ + "' directive requires two ore more arguments.");
    }

    std::string code;
    const std::string path = args[args.size() - 1].literal;
    static const std::string validCodes[] = {"400", "401", "403", "404", "405", "409", "411",
                                             "413", "415", "500", "501", "502", "503", "504"};
    static const size_t validCount = 14;

    for (size_t j = 0; j < args.size() - 1; j++) {
        code = args[j].literal;
        for (size_t i = 0; i < validCount; i++) {
            if (validCodes[i] == code) {
                b.add(code, path);
                break;
            }
            if (i == validCount - 1) {
                throw ConfigError("'" + name_ + " " + code + "' invalid status code.");
            }
        }
    }
}

} // namespace config