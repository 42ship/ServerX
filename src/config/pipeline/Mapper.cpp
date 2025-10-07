#include "config/pipeline/Mapper.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/ConfigNode.hpp"
#include "config/pipeline/DirectiveHandler.hpp"
#include "utils/Logger.hpp"

namespace config {

ServerBlockVec Mapper::map(const ConfigNodeVec &nodes) {
    ServerBlockVec server_blocks;

    if (nodes.empty())
        return server_blocks;
    server_blocks.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); i++) {
        ServerBlock sb;
        mapServerBlock(sb, nodes[i]);
        server_blocks.push_back(sb);
    }
    return server_blocks;
}

void Mapper::mapServerBlock(ServerBlock &b, ConfigNode const &node) {
    DirectiveHandler::getInstance().process(b, node.directives);

    for (size_t i = 0; i < node.children.size(); i++) {
        if (node.children[i].name == "location") {
            handleLocationBlock(b, node.children[i]);
        } else
            LOG_WARN("Unknown block in server block");
    }
}

void Mapper::handleLocationBlock(ServerBlock &b, ConfigNode const &node) {
    LocationBlock lb;

    mapLocationBlock(lb, node);
    if (b.hasLocation(lb)) {
        LOG_WARN("Duplicate location '" << lb.getPath() << "' in location block");
        return;
    }
    details::completeLocationRoot(lb, b);
    b.addLocation(lb);
}

void Mapper::mapLocationBlock(LocationBlock &b, ConfigNode const &node) {
    b.setPath(node.args[0].literal);
    DirectiveHandler::getInstance().process(b, node.directives);
}

} // namespace config
