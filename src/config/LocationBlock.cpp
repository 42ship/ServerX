#include "config/LocationBlock.hpp"
#include "config/internal/Block.hpp"
#include "utils/IndentManager.hpp"

namespace config {

std::string const &LocationBlock::getPath() const { return path_; }

void LocationBlock::setPath(std::string const &v) { path_ = v; }

bool LocationBlock::hasCgiPass() const { return has("cgi_pass"); }

StringVector const *LocationBlock::getIndexFiles() const { return get("index"); }

std::ostream &operator<<(std::ostream &o, const LocationBlock &t) {
    o << print_indent << "[LocationBlock] Path: " << t.getPath() << " {\n";

    o << indent;
    o << static_cast<Block const &>(t);
    o << unindent;

    o << print_indent << "}\n";
    return o;
}

} // namespace config
