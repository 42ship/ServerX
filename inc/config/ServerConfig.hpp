#pragma once

#include "ServerBlock.hpp"
#include <ostream>

namespace http {
class Request;
}

namespace config {

typedef std::map<int, ServerBlockVec> ServerBlockMap;

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
    // ========================= Construction & Destruction =========================

    ServerConfig();

    /**
     * @brief Constructs and builds the configuration from a file path.
     * @param fpath Path to the configuration file.
     * @param perform_fs_checks If true, validates that file paths in the config exist.
     */
    explicit ServerConfig(char const *fpath, bool perform_fs_checks = true);

    /**
     * @brief Constructs and builds the configuration from a string.
     * @param content A string containing the full configuration.
     * @param perform_fs_checks If true, validates that file paths in the config exist.
     */
    explicit ServerConfig(std::string const &content, bool perform_fs_checks = true);

    // ============================== Public Interface ==============================

    /**
     * @brief Retrieves the server configuration that best matches a port and server name.
     * @param port The port number of the incoming connection.
     * @param request The request
     * @return A const pointer to the matched ServerBlock, or NULL if no match is found.
     */
    ServerBlock const *getServer(int port, http::Request const &req) const;

    /**
     * @brief Adds a ServerBlock to the configuration.
     * @param server The ServerBlock to add.
     */
    void addServer(ServerBlock const &server);

    // ============================== Getters & Setters =============================

    /**
     * @brief Gets read-only access to the map of server blocks.
     * @return A constant reference to the server block map.
     */
    ServerBlockMap const &getServersMap() const;

private:
    /**
     * @internal
     * @brief Orchestrates the configuration parsing pipeline.
     * @param content The raw configuration string to parse.
     * @param perform_fs_checks Passed down to the validator.
     */
    void build(std::string const &content, bool perform_fs_checks);

    ServerBlockMap servers_; //!< Map of server blocks, keyed by port number.
};

std::ostream &operator<<(std::ostream &o, ServerConfig const &t);

} // namespace config
