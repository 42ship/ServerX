#include "config/ServerBlock.hpp"
#include <algorithm>

namespace config {
namespace details {

bool matchServerName(std::vector<std::string> const &names, std::string const &s) {
    if (names.empty())
        return true;
    if (std::find(names.begin(), names.end(), s) != names.end())
        return true;
    return false;
}

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
