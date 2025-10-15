#pragma once

#include "types.hpp"

namespace config {

/**
 * @class ConfigNode
 * @brief A generic node representing a block in the configuration file.
 *
 * This class serves as the Intermediate Representation (IR) of the config file.
 * Each ConfigNode can represent a block like 'server' or 'location', holding
 * its own directives and any nested child blocks.
 */
class ConfigNode {
public:
    ConfigNode() { args.reserve(3); };
    ConfigNode(std::string const &conf_name) : name(conf_name) { args.reserve(3); };
    std::string name;
    ParsedDirectiveArgs args;
    ParsedDirectiveMap directives;
    ConfigNodeVec children;
};

} // namespace config
