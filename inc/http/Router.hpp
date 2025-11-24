#pragma once

#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

namespace http {

class MimeTypes;

/**
 * @class Router
 * @brief The central request dispatcher (Facade).
 *
 * This class acts as the main "brain" for handling requests. It uses the
 * ServerConfig to find the correct route, selects the appropriate handler
 * (e.g., StaticFile, CGI), and populates an Response. It also
 * coordinates error page generation.
 */
class Router {
public:
    /**
     * @brief Constructs the Router.
     * @param cfg A constant reference to the global server configuration.
     * @param mime A constant reference to the loaded MIME types.
     */
    Router(config::ServerConfig const &cfg, MimeTypes const &mime);

    /**
     * @brief Populates the request with the matching server and location blocks.
     *
     * This method is intended to be called by the Reactor *before* the body
     * is parsed, allowing for route-specific body validation (e.g., size limits).
     *
     * @param port The port the connection was received on.
     * @param request The HttpRequest object to populate.
     */
    void matchServerAndLocation(int port, Request &request) const;

    /**
     * @brief Dispatches a fully parsed request to the correct handler.
     *
     * This is the main entry point for the Reactor. It finds the route,
     * executes the correct handler, and formats any errors that occur.
     *
     * @param port The port the connection was received on.
     * @param request The fully populated client request.
     * @param response The Response object to be populated by the handler.
     */
    void dispatch(int port, Request const &request, Response &response) const;

private:
    /**
     * @brief Internal: Selects and executes the correct handler.
     * @internal
     * This is called by dispatch() inside a try/catch block.
     *
     * @param request The client request.
     * @param response The Response object to be populated.
     */
    void executeHandler(Request const &request, Response &response) const;

    /**
     * @brief Internal: Populates an error response.
     * @internal
     * Selects the correct ErrorHandler (e.g., JSON or HTML) based on the request.
     *
     * @param request The client request.
     * @param response The Response to populate with an error body.
     */
    static void handleError(Request const &request, Response &response);

    /** @internal */
    config::ServerConfig const &config_;

    // --- Reusable, stateless handler instances ---
    /** @internal */
    StaticFileHandler const staticFile_;
    /** @internal */
    CGIHandler const cgiHandler_;
    /** @internal */
    // FileUploadHandler const uploadHandler_;
    /** @internal */
    // DefaultErrorHandler const defaultErrorHandler_;
    /** @internal */
    // JsonErrorHandler const jsonErrorHandler_;
};

} // namespace http
