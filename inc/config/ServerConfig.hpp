#pragma once

#include <ostream>
#include "ServerBlock.hpp"

namespace config {

/**
 * @class ServerConfig
 * @brief A strongly-typed data container for a server block's configuration.
 *
 * This class holds the final, validated settings for a single 'server' block
 * after all parsing and semantic validation is complete. It is the definitive
 * configuration used by the webserver at runtime.
 */
class ServerConfig {
public:
    ServerConfig(char const *fpath);
    ServerConfig(std::string const &);

    /**
     * @brief Retrieves the server configuration that best matches a port and server name.
     *
     * @param port The port number of the incoming connection.
     * @param server_name The server name requested by the client.
     * @return A const pointer to the matched ServerBlock, or NULL if no match is found.
     */
    ServerBlock const *getServer(int port, std::string const &server_name) const;

    ServerBlockVec const &getServers() const;

    friend std::ostream &operator<<(std::ostream &o, ServerConfig const &t);

private:
    friend class ConfigBuilder;

    ServerBlockVec servers_; // TODO: Change CTL for performance
};

} // namespace config
