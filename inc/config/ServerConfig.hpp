#pragma once

#include "ServerBlock.hpp"
#include <ostream>

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
    ServerConfig(char const *fpath, bool perform_fs_checks = true);
    ServerConfig(std::string const &content, bool perform_fs_checks = true);

    // ============================== Public Interface ==============================
    /**
     * @brief Retrieves the server configuration that best matches a port and server name.
     *
     * @param port The port number of the incoming connection.
     * @param server_name The server name requested by the client.
     * @return A const pointer to the matched ServerBlock, or NULL if no match is found.
     */
    ServerBlock const *getServer(int port, std::string const &server_name) const;
    void addServer(ServerBlock const &);

    // ============================== Getters & Setters =============================

    ServerBlockMap const &getServersMap() const;

private:
    void build(std::string const &content, bool perform_fs_checks);

    ServerBlockMap servers_;
};

std::ostream &operator<<(std::ostream &o, ServerConfig const &t);

} // namespace config
