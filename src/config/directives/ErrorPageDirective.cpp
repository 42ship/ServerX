#include "config/directives/ErrorPageDirective.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <set>
#include <string>

namespace config {
const std::string ErrorPageDirective::name_ = "error_page";

void ErrorPageDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 2, 20, name_); // Arbitrary max to prevent abuse

    const std::string path = args[args.size() - 1].literal;
    static const char *codes[] = {"400", "401", "403", "404", "405", "409", "411",
                                  "413", "415", "500", "501", "502", "503", "504"};
    static const std::set<std::string> validCodes(codes, codes + sizeof(codes) / sizeof(codes[0]));

    for (size_t j = 0; j < args.size() - 1; j++) {
        const std::string &code = args[j].literal;
        ValidatorUtils::checkType(args[j], NUMBER, name_);

        if (validCodes.find(code) == validCodes.end()) {
            throw ConfigError("'" + name_ + " " + code + "' invalid status code.");
        }
        b.add(code, path);
    }
}

} // namespace config