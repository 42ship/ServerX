#pragma once

#include <sstream>

namespace utils {

template <typename T> std::string toString(T const &v) {
    std::ostringstream ss;
    ss << v;
    return ss.str();
}

template <typename T> T fromString(std::string const &str) {
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    return ret;
}

std::string trim(const std::string &s) {
    std::string::size_type start = s.find_first_not_of(" \t");
    std::string::size_type end = s.find_last_not_of(" \t");
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
