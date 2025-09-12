#pragma once

#include "../ServerBlock.hpp"
#include "../LocationBlock.hpp"

namespace config {
/**
 * @class Validator
 * @brief Performs semantic validation on mapped configuration blocks.
 *
 * After the Mapper creates the structure, this class inspects the
 * values to ensure they are valid and logical (e.g., port numbers are in
 * range, required directives are present).
 */
class Validator {
public:
    static void validate(ServerBlockVec const &servers);

private:
    static void validateServer(ServerBlock const &server);
    static void validateLocation(LocationBlock const &location);

    static void validateListen(ServerBlock const &server);
    static void validateRoot(Block const &block);
    static void validateServerNames(ServerBlock const &server);
};

} // namespace config
