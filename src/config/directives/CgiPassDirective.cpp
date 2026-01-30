#include "config/directives/CgiPassDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string CgiPassDirective::name_ = "cgi_pass";

void CgiPassDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "location", name_);
    ValidatorUtils::checkArgs(args, 0, 2, name_);

    if (args.empty()) {
        b.add(name_, "enabled");
    } else if (args.size() == 1) {
        b.add(name_, args[0].literal);
    } else {
        b.add(name_, args[0].literal, args[1].literal);
    }
}

} // namespace config
