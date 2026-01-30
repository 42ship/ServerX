#include "config/directives/AllowMethodsDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string AllowMethodsDirective::name_ = "allow_methods";

void AllowMethodsDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server location", name_);
    ValidatorUtils::checkArgs(args, 1, 3, name_);

    for (size_t i = 0; i < args.size(); i++) {
        std::string const &method = args[i].literal;
        if (method != "GET" && method != "POST" && method != "DELETE" && method != "HEAD") {
            throw ConfigError("'" + name_ + "' invalid method: " + method +
                              ". Allowed: GET, POST, DELETE, HEAD.");
        }
    }

    b.add(name_, args);
}

} // namespace config
