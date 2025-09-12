#pragma once

#include "../internal/ConfigNode.hpp"
#include "../ServerBlock.hpp"
#include "../LocationBlock.hpp"

namespace config {

/**
 * @class Mapper
 * @brief Translates a generic ConfigNode tree into strongly-typed config blocks.
 *
 * This class handles the structural mapping from the parser's output (IR)
 * to the final configuration objects. It does not perform semantic validation.
 */
class Mapper {
public:
    static ServerBlockVec map(ConfigNodeVec const &nodes);

private:
    // Helper methods to map specific blocks and directives
    static void mapServerBlock(ServerBlock &server, ConfigNode const &node);
    static void mapLocationBlock(LocationBlock &location, ConfigNode const &node);
};

} // namespace config
