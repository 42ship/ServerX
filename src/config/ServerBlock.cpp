#include "ServerBlock.hpp"
#include "LocationBlock.hpp"
#include <algorithm>

namespace config {

const int ServerBlock::defaultPort_ = 9191;
const char *ServerBlock::defaultAddress_ = "0.0.0.0";

ServerBlock::ServerBlock() : port_(-1) {
    setDefaultPort();
    setDefaultAddress();
}

LocationBlock const *ServerBlock::getLocation(std::string const &path) const {
    return details::bestMatchLocation(locations_, path);
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
