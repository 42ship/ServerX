#pragma once

#include <sstream>
#include <string>

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

const char *validateDirectoryPath(const char *path);

} // namespace utils
