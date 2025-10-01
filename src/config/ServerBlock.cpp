#include "config/ServerBlock.hpp"
#include "config/internal/Block.hpp"
#include "utils/IndentManager.hpp"
#include <utility>

namespace config {

ServerBlock::ServerBlock() : Block("server"), port_(-1) {}

LocationBlock const *ServerBlock::getLocation(std::string const &path) const {
    return details::bestMatchLocation(locations_, path);
}

bool ServerBlock::hasLocation(LocationBlock const &b) {
    return !b.getPath().empty() && locations_.count(b.getPath());
}

void ServerBlock::addLocation(LocationBlock const &b) {
    std::pair<LocationBlockMap::iterator, bool> result;
    result = locations_.insert(std::make_pair(b.getPath(), b));
    result.first->second.setParent(this);
}

void ServerBlock::setPort(int port) { port_ = port; }
void ServerBlock::setAddress(std::string const &address) { address_ = address; }

std::ostream &operator<<(std::ostream &o, const ServerBlock &t) {
    o << "--- [ServerBlock] --- \n";
    o << "{\n";

    o << indent;

    o << print_indent << "listen address: '" << t.getAddress() << "'\n";
    o << print_indent << "port: '" << t.getPort() << "'\n";

    o << static_cast<Block const &>(t);

    if (!t.locations().empty()) {
        o << "\n" << print_indent << "Locations Defined (" << t.locations().size() << "):\n";
        for (std::map<std::string, LocationBlock>::const_iterator it = t.locations().begin();
             it != t.locations().end(); ++it) {
            o << it->second;
        }
    }

    o << unindent;
    o << "}\n";
    return o;
}

} // namespace config
