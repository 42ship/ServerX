#include "http/Router.hpp"
#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "utils/Logger.hpp"

namespace http {

Router::Router(config::ServerConfig const &cfg, MimeTypes const &mime)
    : config_(cfg), staticFile_(mime) {}

void Router::matchServerAndLocation(int port, Request &request) const {
    request.server(config_.getServer(port, request));
    if (!request.server())
        return;
    request.location(request.server()->matchLocation(request));
}

void Router::dispatch(int port, Request &request, Response &response) const {
    matchServerAndLocation(port, request);
    try {
        executeHandler(request, response);
    } catch (std::exception const &e) {
        LOG_ERROR("Router::dispatch::INTERNAL_SERVER_ERROR: " << e.what());
        response.status(INTERNAL_SERVER_ERROR);
    } catch (...) {
        response.status(INTERNAL_SERVER_ERROR);
    }
    if (response.status() < 400)
        return;
    handleError(request, response);
}

void Router::handleError(Request &request, Response &response) const {
    if (request.wantsJson()) {
        JsonErrorHandler::populateResponse(response);
    } else {
        DefaultErrorHandler::populateResponse(response);
    }
}

void Router::executeHandler(Request &request, Response &response) const {
    if (!request.server())
        response.status(NOT_FOUND);
    else if (!request.location())
        response.status(NOT_FOUND);
    else
        staticFile_.handle(request, response);
#if 0
        else if (request.location->hasCgiPass())
            // TODO: call CGI handler
            ;
        else if (request.requestLine.method == RequestStartLine::POST) {
            // TODO: call file upload handler
            ;
        } else if (request.requestLine.method == RequestStartLine::DELETE) {
            // TODO: call file delte handler
            ;
        } else {
            staticFile_.handle(request, response);
        }
#endif
}

} // namespace http
