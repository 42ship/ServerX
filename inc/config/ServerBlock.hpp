#pragma once

#include "config/Block.hpp"
#include "config/LocationBlock.hpp"

namespace config {

typedef std::map<std::string, LocationBlock> LocationBlockMap;

/**
 * @class ServerBlock
 * @brief Represents a single 'server' block from the configuration file.
 *
 * This class is a strongly-typed container for all the directives and nested
 * location blocks defined within a single server context. It holds settings like
 * the listening port, server names, and a collection of LocationBlock objects.
 */
class ServerBlock : public Block {
public:
    ServerBlock();

    // ============================== Public Interface ==============================

    /**
     * @brief Checks if a location with the same path already exists.
     * @param location The LocationBlock to check.
     * @return True if a location with the same path is already configured, false otherwise.
     */
    bool hasLocation(LocationBlock const &);

    /**
     * @brief Adds a new LocationBlock to this server configuration.
     * @param location The LocationBlock to add.
     */
    void addLocation(LocationBlock const &);

    /**
     * @brief Finds the best-matching location block for a given request URI.
     * @param name The request URI path.
     * @return A const pointer to the matched LocationBlock, or NULL if no match is found.
     */
    LocationBlock const *matchLocation(std::string const &uri) const;

    // ============================== Getters & Setters =============================

    /**
     * @brief Gets the port number this server listens on.
     * @return The integer port number.
     */
    int port() const;

    /**
     * @brief Gets the IP address this server binds to.
     * @return A constant reference to the address string.
     */
    std::string const &address() const;

    // ================================= Fluent API =================================

    /**
     * @brief Sets the port number using a fluent interface.
     * @param p The port number.
     * @return A reference to the ServerBlock object for chaining.
     */
    ServerBlock &port(int port);

    /**
     * @brief Sets the IP address using a fluent interface.
     * @param addr The IP address string.
     * @return A reference to the ServerBlock object for chaining.
     */
    ServerBlock &address(std::string const &address);

private:
    int port_;                   //!< The listening port.
    std::string address_;        //!< The binding IP address.
    LocationBlockMap locations_; //!< Map of configured location blocks, keyed by path.

    friend std::ostream &operator<<(std::ostream &o, ServerBlock const &t);
    friend class Validator;
};

typedef std::vector<ServerBlock> ServerBlockVec;

namespace details {

bool matchServerName(std::vector<std::string> const &, std::string const &);
LocationBlock const *bestMatchLocation(LocationBlockMap const &ls, std::string const &path);

} // namespace details

} // namespace config
