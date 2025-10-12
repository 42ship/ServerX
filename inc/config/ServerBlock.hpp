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

    bool hasLocation(LocationBlock const &);
    void addLocation(LocationBlock const &);

    /**
     * @brief Retrieves the configuration for a specific location path.
     *
     * @param name The request path (URI) to match against a location block.
     * @return A const pointer to the matched LocationBlock, or NULL if no match is found.
     */
    LocationBlock const *matchLocation(std::string const &name) const;

    // ============================== Getters & Setters =============================

    int port() const;
    std::string const &address() const;

    // ================================= Fluent API =================================

    ServerBlock &port(int port);
    ServerBlock &address(std::string const &address);

private:
    int port_;
    std::string address_;
    LocationBlockMap locations_;

    friend std::ostream &operator<<(std::ostream &o, ServerBlock const &t);
    friend class Validator;
};

typedef std::vector<ServerBlock> ServerBlockVec;

namespace details {

bool matchServerName(std::vector<std::string> const &, std::string const &);
LocationBlock const *bestMatchLocation(LocationBlockMap const &ls, std::string const &path);

} // namespace details

} // namespace config
