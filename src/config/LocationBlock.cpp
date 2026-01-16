#include "config/LocationBlock.hpp"
#include "config/Block.hpp"
#include "utils/IndentManager.hpp"

namespace config {

LocationBlock::LocationBlock() : Block("location"), parent_(NULL), matchType_(PREFIX) {}

std::string const &LocationBlock::path() const { return path_; }

LocationBlock::MatchType LocationBlock::matchType() const { return matchType_; }

std::string const &LocationBlock::extension() const { return extension_; }

bool LocationBlock::isRegex() const { return matchType_ == EXTENSION; }

LocationBlock &LocationBlock::path(std::string const &v) {
    path_ = v;
    return *this;
}

LocationBlock &LocationBlock::matchType(MatchType v) {
    matchType_ = v;
    return *this;
}

LocationBlock &LocationBlock::extension(std::string const &v) {
    extension_ = v;
    return *this;
}

bool LocationBlock::hasCgiPass() const { return has("cgi_pass"); }

LocationBlock &LocationBlock::parent(ServerBlock *parent) {
    parent_ = parent;
    return *this;
}

ServerBlock const *LocationBlock::parent() const { return parent_; }

std::ostream &operator<<(std::ostream &o, const LocationBlock &t) {
    o << printIndent << "[LocationBlock] Path: " << t.path();
    if (t.matchType() == LocationBlock::EXTENSION) {
        o << " (extension: " << t.extension() << ")";
    }
    o << " {\n";

    o << indent;
    o << static_cast<Block const &>(t);
    o << unindent;

    o << printIndent << "}\n";
    return o;
}

} // namespace config
