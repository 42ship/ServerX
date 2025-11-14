#include "common/string.hpp"

namespace utils {

std::string trim(const std::string &s) {
    std::string::size_type start = s.find_first_not_of(" \r\t\n");
    std::string::size_type end = s.find_last_not_of(" \r\t\n");
    if (start == std::string::npos) {
        return "";
    }
    return s.substr(start, end - start + 1);
}

bool isAllDigit(std::string const &s) {
    for (std::string::const_iterator it = s.begin(); it != s.end(); ++it) {
        if (!::isdigit(*it))
            return false;
    }
    return true;
}

} // namespace utils
