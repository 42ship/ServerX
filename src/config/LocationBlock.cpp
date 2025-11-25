#include "config/LocationBlock.hpp"
#include "config/Block.hpp"
#include "utils/IndentManager.hpp"

namespace config {

LocationBlock::LocationBlock() : Block("location"), parent_(NULL) {}

std::string const &LocationBlock::path() const { return path_; }

LocationBlock &LocationBlock::path(std::string const &v) {
    path_ = v;
    return *this;
}

bool LocationBlock::hasCgiPass() const { return has("cgi_pass"); }

LocationBlock &LocationBlock::parent(ServerBlock *parent) {
    parent_ = parent;
    return *this;
}

ServerBlock const *LocationBlock::parent() const { return parent_; }

std::ostream &operator<<(std::ostream &o, const LocationBlock &t) {
    o << printIndent << "[LocationBlock] Path: " << t.path() << " {\n";

    o << indent;
    o << static_cast<Block const &>(t);
    o << unindent;

    o << printIndent << "}\n";
    return o;
}

} // namespace config
