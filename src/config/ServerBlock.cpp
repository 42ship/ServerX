#include "config/ServerBlock.hpp"
#include "config/Block.hpp"
#include "utils/IndentManager.hpp"
#include <utility>

namespace config {

ServerBlock::ServerBlock() : Block("server"), port_(-1) {}

LocationBlock const *ServerBlock::matchLocation(std::string const &path) const {
    return details::bestMatchLocation(locations_, path);
}

bool ServerBlock::hasLocation(LocationBlock const &b) {
    return !b.path().empty() && locations_.count(b.path());
}

void ServerBlock::addLocation(LocationBlock const &b) {
    std::pair<LocationBlockMap::iterator, bool> result;
    result = locations_.insert(std::make_pair(b.path(), b));
    result.first->second.parent(this);
}

ServerBlock &ServerBlock::port(int port) {
    port_ = port;
    return *this;
}

int ServerBlock::port() const { return port_; }

ServerBlock &ServerBlock::address(std::string const &address) {
    address_ = address;
    return *this;
}

std::string const &ServerBlock::address() const { return address_; }

std::ostream &operator<<(std::ostream &o, const ServerBlock &t) {
    o << "--- [ServerBlock] --- \n";
    o << "{\n";

    o << indent;

    o << print_indent << "listen address: '" << t.address() << "'\n";
    o << print_indent << "port: '" << t.port() << "'\n";

    o << static_cast<Block const &>(t);

    if (!t.locations_.empty()) {
        o << "\n" << print_indent << "Locations Defined (" << t.locations_.size() << "):\n";
        for (std::map<std::string, LocationBlock>::const_iterator it = t.locations_.begin();
             it != t.locations_.end(); ++it) {
            o << it->second;
        }
    }

    o << unindent;
    o << "}\n";
    return o;
}

} // namespace config
