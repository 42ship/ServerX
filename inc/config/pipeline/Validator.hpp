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
    static void validate(ServerBlockVec &servers, bool perform_fs_checks = true);

private:
    Validator();
    explicit Validator(bool perform_fs_checks = true);
    static void validateServer(ServerBlock &b);
    static void validateGlobalConstraints(ServerBlockVec const &servers);

    bool perform_fs_checks_;
};

} // namespace config
