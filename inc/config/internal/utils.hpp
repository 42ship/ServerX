#pragma once

#include <string>
#include <sstream>

namespace utils {

bool isValidPort(int port);
bool isValidPort(std::string const &);
bool isValidIPv4(char const *);
bool isValidIPv4(std::string const &);
bool isAllDigit(std::string const &s);

struct IpInfo {
    std::string ip;
    int port;
};

bool extractIpInfo(std::string const &, IpInfo &);

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

} // namespace utils
