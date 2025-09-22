#pragma once

#include "../LocationBlock.hpp"
#include "../ServerBlock.hpp"

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
    static void validate(ServerBlockVec &servers);

private:
    static void validateServer(ServerBlock &server);
    static void validateLocation(LocationBlock &location, ServerBlock const &server);

    static void validateListen(ServerBlock &server);
    static void validateRoot(Block &block);
    static void validateServerNames(ServerBlock &server);

    static void locationCompleteRoot(LocationBlock &l, ServerBlock const &s);
};

} // namespace config
