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

    /**
     * @brief Checks if a 'cgi_pass' directive is configured for this location.
     * @return True if 'cgi_pass' is present, false otherwise.
     */
    bool hasCgiPass() const;

    // ============================== Getters & Setters =============================

    /**
     * @brief Gets the URI path that this location block matches.
     * @return A constant reference to the path string.
     */
    std::string const &path() const;

    /**
     * @brief Gets a pointer to the parent ServerBlock.
     * @return A constant pointer to the parent server configuration block.
     */
    ServerBlock const *parent() const;

    // ================================= Fluent API =================================

    /**
     * @brief Sets the URI path for this location using a fluent interface.
     * @param p The path to set (e.g., "/images/").
     * @return A reference to the LocationBlock object for chaining.
     */
    LocationBlock &path(std::string const &);

    /**
     * @brief Sets the parent ServerBlock for this location.
     * @param p A pointer to the parent ServerBlock.
     * @return A reference to the LocationBlock object for chaining.
     */
    LocationBlock &parent(ServerBlock *parent);

private:
    ServerBlock *parent_; //!< Pointer to the parent ServerBlock.
    std::string path_;    //!< The URI path this location matches.
};

std::ostream &operator<<(std::ostream &o, LocationBlock const &t);

} // namespace config
