#include "config/ServerBlock.hpp"
#include "config/ConfigException.hpp"
#include "config/internal/utils.hpp"

#include <algorithm>

namespace config {

namespace {
bool parseListen(const std::string &s, std::string &addr_part, std::string &port_part);
const int defaultPort_ = 9191;
const char *defaultAddress_ = "0.0.0.0";
} // namespace

ServerBlock::ServerBlock() : port_(defaultPort_), address_(defaultAddress_) {}

LocationBlock const *ServerBlock::getLocation(std::string const &path) const {
    return details::bestMatchLocation(locations_, path);
}

bool ServerBlock::hasLocation(LocationBlock const &b) {
    return !b.getPath().empty() && locations_.count(b.getPath());
}

void ServerBlock::addLocation(LocationBlock const &b) { locations_[b.getPath()] = b; }

void ServerBlock::setListen(std::string const &listenArg) {
    std::string port_str;
    std::string address;
    if (!parseListen(listenArg, address, port_str)) {
        throw ConfigError("Listen directive '" + listenArg + "' has an invalid format.");
    }
    if (!utils::isValidPort(port_str)) {
        throw ConfigError("Port '" + port_str + "' is not valid.");
    }
    port_ = utils::fromString<int>(port_str);
    address_ = address;
}

std::ostream &operator<<(std::ostream &o, const ServerBlock &t) {
    o << "--- [ServerBlock] --- \n";
    o << "{\n";
    o << "    Listen Address: '" << t.getAddress() << "'\n";
    o << "    Port:           " << t.getPort() << "\n";

    // TODO: add printing for directives

    if (!t.locations().empty()) {
        o << "\n    Locations Defined (" << t.locations().size() << "):\n";
        for (std::map<std::string, LocationBlock>::const_iterator it = t.locations().begin();
             it != t.locations().end(); ++it) {
            o << it->second;
        }
    }

    o << "}\n";
    return o;
}

namespace details {

bool matchServerName(std::vector<std::string> const &names, std::string const &s) {
    if (names.empty())
        return true;
    if (std::find(names.begin(), names.end(), s) != names.end())
        return true;
    return false;
}

} // namespace details

namespace {

bool parseListen(const std::string &s, std::string &addr_part, std::string &port_part) {
    if (s.empty())
        return false;

    size_t cpos = s.find_last_of(':');

    if (cpos == std::string::npos) {
        bool is_numeric = (s.find_first_not_of("0123456789") == std::string::npos);
        if (is_numeric) {
            port_part = s;
        } else {
            addr_part = s;
        }
    } else {
        addr_part = s.substr(0, cpos);
        port_part = s.substr(cpos + 1);

        if (addr_part.empty() || port_part.empty())
            return false;
    }
    return true;
}

} // namespace

} // namespace config
