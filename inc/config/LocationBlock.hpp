#pragma once

#include "config/Block.hpp"
#include <ostream>

namespace config {

class ServerBlock;

/**
 * @class LocationBlock
 * @brief Represents a single 'location' block from the configuration file.
 *
 * This class is a strongly-typed container for the directives defined within a
 * location context. It holds settings such as the document root and index files
 * for a specific URI path.
 */
class LocationBlock : public Block {
public:
    LocationBlock();

    // ============================== Public Interface ==============================

    bool hasCgiPass() const;

    // ============================== Getters & Setters =============================

    std::string const &path() const;
    ServerBlock const *parent();

    // ================================= Fluent API =================================

    LocationBlock &path(std::string const &);
    LocationBlock &parent(ServerBlock *parent);

private:
    ServerBlock *parent_;
    std::string path_;
};

std::ostream &operator<<(std::ostream &o, LocationBlock const &t);

} // namespace config
