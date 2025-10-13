#pragma once

#include "LocationBlock.hpp"
#include "internal/Block.hpp"

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
    // ========================= Construction & Destruction =========================

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
    LocationBlock const *getLocation(std::string const &name) const;
    LocationBlockMap const &locations() const { return locations_; }
    LocationBlockMap &locations() { return locations_; }

    // ============================== Getters & Setters =============================

    void setPort(int port);
    int getPort() const { return port_; }
    void setAddress(std::string const &address);
    std::string const &getAddress() const { return address_; }

private:
    int port_;
    std::string address_;
    LocationBlockMap locations_;
};

typedef std::vector<ServerBlock> ServerBlockVec;

std::ostream &operator<<(std::ostream &o, ServerBlock const &t);

namespace details {

bool matchServerName(std::vector<std::string> const &, std::string const &);
LocationBlock const *bestMatchLocation(LocationBlockMap const &ls, std::string const &path);

} // namespace details

} // namespace config
