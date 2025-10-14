#include "config/internal/utils.hpp"
#include "common/string.hpp"

#include <arpa/inet.h>

using namespace std;

namespace utils {

bool isValidIPv4(std::string const &s) {
    sockaddr_in sk;
    return !s.empty() && s.size() >= 7 && s.size() <= 15 &&
           inet_pton(AF_INET, s.c_str(), &sk.sin_addr) == 1;
}

bool isValidPort(int port) { return (port > 0 && port <= 65535); }

bool isValidPort(std::string const &v) {
    return !v.empty() && v.size() <= 5 && isAllDigit(v) && isValidPort(fromString<int>(v));
}

bool extractIpInfo(std::string const &s, IpInfo &info) {
    if (s.empty())
        return false;
    info.ip.clear();
    info.port = -1;
    size_t cpos = s.find(':');
    if (cpos == string::npos) {
        if (isValidPort(s)) {
            info.port = fromString<int>(s);
        } else if (isValidIPv4(s)) {
            info.ip = s;
        } else if (s == "localhost") {
            info.ip = "127.0.0.1";
        } else
            return false;
        return true;
    }
    std::string port_part = s.substr(cpos + 1);
    std::string ip_part = s.substr(0, cpos);
    if (ip_part == "localhost")
        ip_part = "127.0.0.1";
    if (!isValidIPv4(ip_part) || !isValidPort(port_part))
        return false;
    info.ip = ip_part;
    info.port = fromString<int>(port_part);
    return true;
}

} // namespace utils
