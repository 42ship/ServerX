#include "config/LocationBlock.hpp"

namespace config {

std::ostream &operator<<(std::ostream &o, const LocationBlock &t) {
    o << "    [LocationBlock] Path: " << t.path << "\n";
    o << "    {\n";
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
    }
    // Add other location directives here...
    o << "    }\n";
    return o;
}

} // namespace config
