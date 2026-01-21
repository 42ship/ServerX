#include "config/directives/ErrorPageDirective.hpp"

#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/String.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include <set>
#include <string>

namespace config {
const std::string ErrorPageDirective::name_ = "error_page";

void ErrorPageDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.size() < 2) {
        throw ConfigError("'" + name_ + "' directive requires two or more arguments.");
    }

    const std::string path = args[args.size() - 1].literal;
    static const char *codes[] = {"400", "401", "403", "404", "405", "409", "411",
                                  "413", "415", "500", "501", "502", "503", "504"};
    static const std::set<std::string> validCodes(codes, codes + sizeof(codes) / sizeof(codes[0]));

    for (size_t j = 0; j < args.size() - 1; j++) {
        const std::string &code = args[j].literal;
        if (validCodes.find(code) == validCodes.end()) {
            throw ConfigError("'" + name_ + " " + code + "' invalid status code.");
        }
        b.add(code, path);
    }
}

} // namespace config