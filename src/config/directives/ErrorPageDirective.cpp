#include "config/directives/ErrorPageDirective.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <string>

namespace config {
const std::string ErrorPageDirective::name_ = "error_page";

void ErrorPageDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 2, 20, name_); // Arbitrary max to prevent abuse

    const std::string path = args[args.size() - 1].literal;
    static const std::string validCodes[] = {"400", "401", "403", "404", "405", "409", "411",
                                             "413", "415", "500", "501", "502", "503", "504"};
    static const size_t validCount = sizeof(validCodes) / sizeof(validCodes[0]);

    for (size_t j = 0; j < args.size() - 1; j++) {
        const std::string &code = args[j].literal;
        ValidatorUtils::checkType(args[j], NUMBER, name_);

        bool isFound = false;
        for (size_t i = 0; i < validCount; i++) {
            if (validCodes[i] == code) {
                b.add(code, path);
                isFound = true;
                break;
            }
        }
        if (!isFound) {
            throw ConfigError("'" + name_ + " " + code + "' invalid status code.");
        }
    }
}

} // namespace config