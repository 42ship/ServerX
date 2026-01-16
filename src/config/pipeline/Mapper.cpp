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
        LOG_WARN("Duplicate location '" << lb.path() << "' in location block");
        return;
    }
    details::completeLocationRoot(lb, b);
    b.addLocation(lb);
}

void Mapper::mapLocationBlock(LocationBlock &b, ConfigNode const &node) {
    if (node.args.size() == 2 && node.args[0].literal == "~") {
        std::string pattern = node.args[1].literal;
        b.path(pattern);
        if (pattern.size() > 3 && pattern.substr(0, 2) == "\\." &&
            pattern[pattern.size() - 1] == '$') {
            b.matchType(LocationBlock::EXTENSION);
            b.extension(pattern.substr(1, pattern.size() - 2)); // Extract ".ext"
        } else {
            b.matchType(LocationBlock::PREFIX);
        }
    } else {
        b.path(node.args[0].literal);
        b.matchType(LocationBlock::PREFIX);
    }
    DirectiveHandler::getInstance().process(b, node.directives);
}

} // namespace config
