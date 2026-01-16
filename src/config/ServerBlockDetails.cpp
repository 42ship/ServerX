#include "config/ServerBlock.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <regex.h>
#include <vector>

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

LocationBlock const *matchExtensionLocation(LocationBlockMap const &ls,
                                            std::vector<std::string> const &paths,
                                            std::string const &uri) {
    for (size_t i = 0; i < paths.size(); ++i) {
        LocationBlockMap::const_iterator it = ls.find(paths[i]);
        if (it == ls.end())
            continue;

        const std::string &extension = it->second.extension();
        if (uri.size() >= extension.size() &&
            uri.compare(uri.size() - extension.size(), extension.size(), extension) == 0) {
            LOG_TRACE("matchExtensionLocation: matched " << paths[i]);
            return &it->second;
        }
    }
    return NULL;
}

} // namespace details

} // namespace config
