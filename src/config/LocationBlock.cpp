#include "config/LocationBlock.hpp"

namespace config {

std::ostream &operator<<(std::ostream &o, LocationBlock const &t) {
    o << "\t\tlocation " << t.path << " {\n";
    o << "\t\t\troot: " << t.root << ";\n";
    o << "\t\t\tindex:";
    for (std::vector<std::string>::const_iterator it = t.index.begin(); it != t.index.end(); ++it) {
        o << " " << *it;
    }
    o << ";\n\t\t}\n";
    return o;
}

} // namespace config
