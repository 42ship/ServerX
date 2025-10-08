#include "common/string.hpp"

namespace utils {

std::string trim(const std::string &s) {
    std::string::size_type start = s.find_first_not_of(" \t");
    std::string::size_type end = s.find_last_not_of(" \t");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, end - start + 1);
}

} // namespace utils
