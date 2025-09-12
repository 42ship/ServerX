#include "config/pipeline/Mapper.hpp"
#include "config/internal/ConfigNode.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "utils/Logger.hpp"

namespace config {

ServerBlockVec Mapper::map(const ConfigNodeVec &nodes) {
    ServerBlockVec cb(nodes.size());

    if (nodes.empty())
        return cb;
    for (size_t i = 0; i < nodes.size(); i++) {
        ServerBlock sb;
        mapServerBlock(sb, nodes[i]);
        cb.push_back(sb);
    }
    return cb;
}

void Mapper::mapServerBlock(ServerBlock &b, ConfigNode const &node) {
    for (size_t i = 0; i < node.children.size(); i++) {
        if (node.children[i].name == "location") {
            LocationBlock lb;
            mapLocationBlock(lb, node.children[i]);
            if (b.locations_.count(lb.path_)) {
                LOG_WARN("Duplicate location '" << lb.path_ << "' in location block");
                continue;
            }
            b.locations_[lb.path_] = lb;
        } else
            LOG_WARN("Unknown block in server block");
    }
}

void Mapper::mapLocationBlock(LocationBlock &b, ConfigNode const &node) {
    b.path_ = node.args[0];
    b.directives_.insert(node.directives.begin(), node.directives.end());
}

} // namespace config
