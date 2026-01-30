#include "config/directives/UploadPathDirective.hpp"
#include "config/internal/ValidationUtils.hpp"

namespace config {

const std::string UploadPathDirective::name_ = "upload_path";

void UploadPathDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    ValidatorUtils::checkContext(b, "location", name_);
    ValidatorUtils::checkArgs(args, 1, 1, name_);

    std::string upload = args[0].literal;
    if (!upload.empty() && upload[0] == '/') {
        throw ConfigError("'" + name_ +
                          "' directive requires a relative path; absolute paths (starting with "
                          "'/') are not permitted.");
    }

    b.add(name_, args);
}

} // namespace config
