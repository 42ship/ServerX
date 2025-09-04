#include "ConfigException.hpp"
#include <iostream>

namespace config {

// TODO: use global variable instead of macro
void issue_warning(std::string const &msg) {
#ifdef WERROR
    throw ConfigWarning(msg);
#else
    std::cerr << YELLOW "Warning: " NC << msg << std::endl;
#endif
}

} // namespace config
