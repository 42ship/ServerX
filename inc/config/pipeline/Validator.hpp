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
    Validator(bool perform_fs_checks = true);
    void validateServer(ServerBlock &server);
    void validateLocation(LocationBlock &location, ServerBlock const &server);

    void validateListen(ServerBlock &server);
    void validateRoot(Block &block);
    void validateServerNames(ServerBlock &server);

    static void locationCompleteRoot(LocationBlock &l, ServerBlock const &s);

    bool perform_fs_checks_;
};

} // namespace config
