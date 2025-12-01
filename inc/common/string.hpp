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

template <typename T> T fromStringSafe(std::string const &str) {
    std::istringstream ss(str);
    T ret;
    ss >> ret;
    if (ss.fail() || !ss.eof())
        throw std::runtime_error("Invalid number format: '" + str + "'");
    return ret;
}

std::string trim(const std::string &s);

bool isAllDigit(std::string const &s);

} // namespace utils
