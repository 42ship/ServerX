#include "config/directives/ReturnDirective.hpp"
#include "common/string.hpp"
#include "config/internal/ValidationUtils.hpp"
#include <string>

namespace config {

const std::string ReturnDirective::name_ = "return";

void ReturnDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 1, 2, name_);

    if (args.size() == 1) {
        // Syntax: return code; or return URL;
        if (args[0].type == NUMBER) {
            int code = utils::fromString<int>(args[0].literal);
            if (code < 200 || code >= 600)
                throw ConfigError("'" + name_ + "' invalid status code: " + args[0].literal);
            if (code >= 300 && code < 400)
                throw ConfigError("'" + name_ + "' status " + args[0].literal + " requires a URL.");
            b.add(name_, args);
        } else {
            // Treat as return 302 URL;
            b.add(name_, "302", args[0].literal);
        }
        return;
    }

    // Syntax: return code [text|URL];
    ValidatorUtils::checkType(args[0], NUMBER, name_);
    int code = utils::fromString<int>(args[0].literal);
    if (code < 200 || code >= 600)
        throw ConfigError("'" + name_ + "' invalid status code: " + args[0].literal);

    if (code >= 300 && code < 400) {
        // Redirection must be a URL
    }
    b.add(name_, args);
}

} // namespace config
