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
    static void mapServerBlock(ServerBlock &, ConfigNode const &);
    static void mapLocationBlock(LocationBlock &, ConfigNode const &);

    static void handleLocationBlock(ServerBlock &, ConfigNode const &);
};

} // namespace config
