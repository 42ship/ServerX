#pragma once

#include <vector>
#include <string>
#include <map>

#include "LocationBlock.hpp"

namespace config {

/**
 * @class ServerBlock
 * @brief Represents a single 'server' block from the configuration file.
 *
 * This class is a strongly-typed container for all the directives and nested
 * location blocks defined within a single server context. It holds settings like
 * the listening port, server names, and a collection of LocationBlock objects.
 */
class ServerBlock {
public:
    ServerBlock();

    /**
     * @brief Retrieves the configuration for a specific location path.
     *
     * @param name The request path (URI) to match against a location block.
     * @param[out] res A reference to a pointer that will be set to the matched LocationBlock.
     * @return True if a matching location is found, false otherwise.
     */
    bool getLocation(std::string const &name, LocationBlock const *&res) const;

    /**
     * @brief Retrieves the configuration for a specific location path.
     *
     * @param name The request path (URI) to match against a location block.
     * @return A const pointer to the matched LocationBlock, or NULL if no match is found.
     */
    LocationBlock const *getLocation(std::string const &name) const;
    bool matchServerName(std::string const &) const;

private:
    void setDefaultPort();
    void setDefaultAddress();

    friend class ConfigBuilder;
    friend class ServerConfig;

    static const int defaultPort_;
    static const char *defaultAddress_;

    int port_;
    std::string address_;
    std::vector<std::string> serverNames_; // TODO: Change CTL for performance
    std::map<std::string, LocationBlock> locations_;
};

typedef std::vector<ServerBlock> ServerBlockVec;

namespace details {

bool matchServerName(std::vector<std::string> const &, std::string const &);

}

} // namespace config
