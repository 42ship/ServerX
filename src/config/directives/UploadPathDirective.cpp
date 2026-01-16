#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/directives/UploadPathDirective.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include <string>

namespace config {

const std::string UploadPathDirective::name_ = "upload_path";

void UploadPathDirective::process(Block &b, ParsedDirectiveArgs const &args) const {
    if (args.size() != 1)
        throw ConfigError("'upload_path' directive requires exactly one argument.");
    std::string upload = args[0].literal;
    if (!dynamic_cast<LocationBlock *>(&b))
        throw ConfigError("'" + name_ + "' directive is not allowed in: " + b.name());

    if (!upload.empty() && upload[0] == '/') {
        throw ConfigError("'" + name_ +
                          "' directive requires a relative path; absolute paths (starting with "
                          "'/') are not permitted.");
    }
    b.add(name_, upload);
}

} // namespace config
