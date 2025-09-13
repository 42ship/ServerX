#include "config/pipeline/Mapper.hpp"
#include "config/pipeline/DirectiveHandler.hpp"
#include "config/internal/ConfigNode.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
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
    DirectiveHandler &i = DirectiveHandler::getInstance();
    i.process(b, node.directives);
    // DirectiveHandler::getInstance().process(b, node.directives);

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
    if (b.locations_.count(lb.path_)) {
        LOG_WARN("Duplicate location '" << lb.path_ << "' in location block");
        return;
    }
    b.locations_[lb.path_] = lb;
}

void Mapper::mapLocationBlock(LocationBlock &b, ConfigNode const &node) {
    b.path_ = node.args[0];
    DirectiveHandler::getInstance().process(b, node.directives);
}

} // namespace config
