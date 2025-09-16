#include "config/LocationBlock.hpp"
#include "config/internal/Block.hpp"

namespace config {

std::string const &LocationBlock::getPath() const {
    return path_;
}

void LocationBlock::setPath(std::string const &v) {
    path_ = v;
}

bool LocationBlock::hasCgiPass() const {
    return has("cgi_pass");
}

StringVector const *LocationBlock::getIndexFiles() const {
    return get("index");
}

std::ostream &operator<<(std::ostream &o, const LocationBlock &t) {
    o << "    [LocationBlock] Path: " << t.getPath() << "\n";
    o << "    {\n";
    /*
    if (!t.root.empty()) {
        o << "        root:  '" << t.root << "'\n";
    }
    if (!t.index.empty()) {
        o << "        index: [";
        for (std::vector<std::string>::const_iterator it = t.index.begin(); it != t.index.end();
             ++it) {
            o << "'" << *it << "'";
            if (it + 1 != t.index.end()) {
                o << ", ";
            }
        }
        o << "]\n";
    }*/
    // Add other location directives here...
    o << "    }\n";
    return o;
}

} // namespace config
