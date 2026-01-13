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
    /**
     * @brief Specifies how the location path should be matched against the request URI.
     */
    enum MatchType {
        PREFIX,   //!< Standard prefix matching (e.g., /static/)
        EXTENSION //!< File extension matching (e.g., .php)
    };

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
     * @brief Gets the match type for this location.
     * @return The MatchType (PREFIX or EXTENSION).
     */
    MatchType matchType() const;

    /**
     * @brief Gets the extension string (including the dot) for EXTENSION match type.
     * @return The extension string (e.g., ".php").
     */
    std::string const &extension() const;

    /**
     * @brief Compatibility wrapper for checking if this is a "regex" (extension) location.
     * @return true if matchType is EXTENSION.
     */
    bool isRegex() const; // For backward compatibility/API consistency

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
     * @brief Sets the match type for this location.
     * @return A reference to the LocationBlock object for chaining.
     */
    LocationBlock &matchType(MatchType);

    /**
     * @brief Sets the extension string for EXTENSION match type.
     * @param ext The extension including the dot (e.g., ".php").
     * @return A reference to the LocationBlock object for chaining.
     */
    LocationBlock &extension(std::string const &);

    /**
     * @brief Sets the parent ServerBlock for this location.
     * @param p A pointer to the parent ServerBlock.
     * @return A reference to the LocationBlock object for chaining.
     */
    LocationBlock &parent(ServerBlock *parent);

private:
    ServerBlock *parent_;   //!< Pointer to the parent ServerBlock.
    std::string path_;      //!< The URI path/pattern this location matches.
    MatchType matchType_;   //!< How this location matches.
    std::string extension_; //!< The extracted extension if MatchType is EXTENSION.
};

std::ostream &operator<<(std::ostream &o, LocationBlock const &t);

} // namespace config
