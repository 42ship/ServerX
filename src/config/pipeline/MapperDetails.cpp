#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"

namespace config {

namespace details {

void completeLocationRoot(LocationBlock &l, ServerBlock const &s) {
    std::string finalRoot;

    std::string serverRoot = s.root();
    std::string locationRoot = l.root();
    if (!locationRoot.empty()) {
        // root is an absolute path
        if (locationRoot[0] == '/') {
            finalRoot = locationRoot;
        } else { // root is a relative path
            finalRoot = serverRoot;
            if (!finalRoot.empty() && finalRoot[finalRoot.length() - 1] != '/') {
                finalRoot += '/';
            }
            finalRoot += locationRoot;
        }
    } else {
        finalRoot = serverRoot;
    }
    if (finalRoot.empty()) {
        return;
    }
    l.root(finalRoot);
}

} // namespace details
} // namespace config
