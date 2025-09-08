#include "config/ServerBlock.hpp"
#include <iostream>

namespace config {
namespace details {

LocationBlock const *bestMatchLocation(LocationBlockMap const &ls, std::string const &path) {
    std::string currentPath = path;

    while (!currentPath.empty()) {
        LocationBlockMap::const_iterator it = ls.find(currentPath);
        if (it != ls.end()) {
            return &it->second;
        }
        if (currentPath == "/")
            break;
        size_t pos = currentPath.rfind('/', currentPath.size() - 2);
        if (pos != std::string::npos) {
            currentPath.resize(pos + 1);
        } else
            break;
    }
    LocationBlockMap::const_iterator it = ls.find("/");
    if (it != ls.end())
        return &it->second;
    return NULL;
}

} // namespace details
} // namespace config
