#pragma once

#include <string>

#include "utils/utils.hpp"

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

} // namespace utils
