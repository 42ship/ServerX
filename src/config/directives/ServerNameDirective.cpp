#include "config/directives/ServerNameDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string ServerNameDirective::name_ = "server_name";

void ServerNameDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "server", name_);
    ValidatorUtils::checkArgs(args, 1, 10, name_);

    // Nginx style validation for hostnames could be added here if needed
    b.add(name_, args);
}

} // namespace config
