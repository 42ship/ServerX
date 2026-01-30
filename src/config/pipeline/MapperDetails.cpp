#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"

namespace config {

namespace details {

void completeLocationRoot(LocationBlock &l, ServerBlock const &s) {
    std::string locationRoot = l.root();

    if (locationRoot.empty()) {
        std::string serverRoot = s.root();
        l.root(serverRoot);
    } else {
        // If location has root set, it strictly overrides server root.
        // In our implementation, we treat it as-is (could be relative to CWD
        // or absolute, but we don't concatenate it with serverRoot anymore).
        l.root(locationRoot);
    }
}

} // namespace details
} // namespace config
