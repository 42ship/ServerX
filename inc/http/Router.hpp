#pragma once

#include "http/Handler.hpp"
#include "http/HttpRequest.hpp"
#include "http/MimeTypes.hpp"
#include "http/RouterResult.hpp"
#include "config/ServerConfig.hpp"

namespace http {

/**
 * @class Router
 * @brief The central request dispatcher.
 *
 * Matches an incoming HttpRequest to a specific handler based on the server
 * configuration.
 */
class Router {
public:
    Router(config::ServerConfig const &config, MimeTypes const &mime)
        : config_(config), staticFile_(mime) {
    }

    /**
     * @brief Determines the correct handler and context for a request.
     * @param port The port the connection was received on.
     * @param request The client's HTTP request.
     * @return A RouterResult containing the matched handler and config blocks.
     */
    RouterResult route(int port, HttpRequest &request) const {
        config::ServerBlock const *server = config_.getServer(port, request.headers["Host"]);
        if (request.status != 200) {
            return RouterResult(error_, server);
        }
        if (!server) {
            return RouterResult(notFound_);
        }
        config::LocationBlock const *location = server->getLocation(request.path);
        if (!location) {
            return RouterResult(notFound_, server);
        }
        if (location->hasCgiPass())
            return RouterResult(cgi_, server, location);
        return RouterResult(staticFile_, server, location);
    }

private:
    config::ServerConfig const &config_;

    // Reusable, stateless handler instances owned by the router.
    NotFoundHandler const notFound_;
    StaticFileHandler const staticFile_;
    CGIHandler const cgi_;
    DefaultErrorHandler const error_;
};

} // namespace http
