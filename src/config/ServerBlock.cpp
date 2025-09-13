#include "config/ServerBlock.hpp"

#include <algorithm>

namespace config {

const int ServerBlock::defaultPort_ = 9191;
const char *ServerBlock::defaultAddress_ = "0.0.0.0";

ServerBlock::ServerBlock() : port_(defaultPort_), address_(defaultAddress_) {
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

int ServerBlock::getPort() const {
    return port_;
}
std::string const &ServerBlock::getAddress() const {
    return address_;
}

std::ostream &operator<<(std::ostream &o, const ServerBlock &t) {
    o << "--- [ServerBlock] --- \n";
    o << "{\n";
    o << "    Listen Address: '" << t.getAddress() << "'\n";
    o << "    Port:           " << t.getPort() << "\n";

    /*
    if (!t.serverNames_.empty()) {
        o << "    Server Names:   [";
        const std::vector<std::string> &names = t.serverNames_;
        for (std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            o << "'" << *it << "'";
            if (it + 1 != names.end()) {
                o << ", ";
            }
        }
        o << "]\n";
    }

    if (!t.root.empty()) {
        o << "    Default Root:   '" << t.root << "'\n";
    }
    */
    if (!t.locations_.empty()) {
        o << "\n    Locations Defined (" << t.locations_.size() << "):\n";
        for (std::map<std::string, LocationBlock>::const_iterator it = t.locations_.begin();
             it != t.locations_.end(); ++it) {
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

} // namespace config
