#include "ServerBlock.hpp"
#include <algorithm>

namespace config {

const int ServerBlock::defaultPort_ = 9191;
const char *ServerBlock::defaultAddress_ = "0.0.0.0";

ServerBlock::ServerBlock() : port_(-1) {
    setDefaultPort();
    setDefaultAddress();
}

bool ServerBlock::getLocation(std::string const &name, LocationBlock const *&res) const {
    std::map<std::string, LocationBlock>::const_iterator it = locations_.find(name);
    if (it == locations_.end())
        return false;
    res = &it->second;
    return true;
}

LocationBlock const *ServerBlock::getLocation(std::string const &server_name) const {
    LocationBlock const *res;
    if (getLocation(server_name, res)) {
        return res;
    }
    return NULL;
}

// TODO: proper host matching
bool ServerBlock::matchServerName(std::string const &needle) const {
    return details::matchServerName(serverNames_, needle);
}

void ServerBlock::setDefaultPort() {
    port_ = defaultPort_;
}
void ServerBlock::setDefaultAddress() {
    address_ = defaultAddress_;
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

} // namespace config
