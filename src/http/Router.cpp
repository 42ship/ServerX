#include "http/Router.hpp"
#include "common/string.hpp"
#include "config/ServerConfig.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/handlers/CGIHandler.hpp"
#include "http/handlers/DirectoryListingHandler.hpp"
#include "utils/Logger.hpp"
#include <sys/stat.h>
#include <unistd.h>

namespace http {

Router::Router(config::ServerConfig const &cfg, MimeTypes const &mime)
    : config_(cfg), mimeTypes_(mime) {
    LOG_TRACE("Router::Router(): router created");
}

void Router::matchServerAndLocation(int port, Request &request) const {
    std::string ctx = "(" + utils::toString(port) + ", " + request.path() + "): ";
    (void)ctx;
    request.server(config_.getServer(port, request));
    if (!request.server()) {
        LOG_STRACE(ctx + "No matching server found");
        return;
    }
    LOG_STRACE(ctx + "Matched server=" + request.server()->name());
    request.location(request.server()->matchLocation(request));
    if (!request.location()) {
        LOG_STRACE(ctx + "No matching location found");
    } else {
        LOG_STRACE(ctx + "Matched location=" + request.location()->path());
    }
}

void Router::dispatch(int port, Request const &request, Response &response) const {
    (void)port;
    LOG_DEBUG("Router::dispatch(" << port << ", "
                                  << RequestStartLine::methodToString(request.method()) << " "
                                  << request.uri() << "): dispatching request");
    if (request.status() >= 400) {
        response.status(request.status());
        return handleError(request, response, this->mimeTypes_);
    }
    try {
        executeHandler(request, response);
    } catch (std::exception const &e) {
        LOG_ERROR("Router::dispatch(" << port << ", " << request.uri()
                                      << "): exception in executeHandler: " << e.what());
        response.status(INTERNAL_SERVER_ERROR, e.what());
    } catch (...) {
        LOG_ERROR("Router::dispatch(" << port << ", " << request.uri()
                                      << "): unknown exception in executeHandler");
        response.status(INTERNAL_SERVER_ERROR);
    }
    if (response.status() < 400) {
        LOG_DEBUG("Router::dispatch("
                  << port << ", " << request.uri()
                  << "): handler finished successfully. status=" << response.status());
        return;
    }
    LOG_DEBUG("Router::dispatch(" << port << ", " << request.uri()
                                  << "): handler reported error. status=" << response.status()
                                  << ", formatting error response...");
    handleError(request, response, this->mimeTypes_);
}

void Router::handleError(Request const &request, Response &response, MimeTypes const &mimeTypes) {
    if (request.wantsJson()) {
        LOG_TRACE("Router::handleError(" << request.uri() << "): populating JSON error page");
        JsonErrorHandler::populateResponse(response);
    } else {
        LOG_TRACE("Router::handleError(" << request.uri()
                                         << "): populating default HTML error page");
        DefaultErrorHandler::handle(request, response, mimeTypes);
    }
}

void Router::handleError(Request const &request, Response &response) const {
    handleError(request, response, mimeTypes_);
}

void Router::executeHandler(Request const &request, Response &response) const {
    std::string ctx = "'" + request.uri() + "': ";
    (void)ctx;
    if (!request.server()) {
        LOG_STRACE(ctx << "No server found, setting 404");
        response.status(NOT_FOUND);
    } else if (!request.location()) {
        LOG_STRACE(ctx << "No location found, setting 404");
        response.status(NOT_FOUND);
    } else if (request.location()->hasCgiPass()) {
        LOG_STRACE(ctx << "Dispatched to CGIHandler");
        CGIHandler::handle(request, response, mimeTypes_);
    } else if (request.location()->has("return")) {
        LOG_STRACE(ctx << "Dispatched to ReturnHandler");
        ReturnHandler::handle(request, response);
    } else if (request.method() == RequestStartLine::DELETE) {
        LOG_STRACE(ctx << "Dispatched to FileDeleteHandler");
        FileDeleteHandler::handle(request, response);
    } else if (request.method() == RequestStartLine::POST) {
        LOG_TRACE("Router::executeHandler(" << request.uri()
                                            << "): dispatching to FileUploadHandler");
        FileUploadHandler::handle(request, response, mimeTypes_);
    } else if (request.location()->autoIndex()) {
        LOG_STRACE(ctx << "Dispatched to DirectoryListingHandler");
        DirectoryListingHandler::handle(request, response, mimeTypes_);
    } else {
        LOG_STRACE(ctx << "Dispatched to StaticFileHandler");
        StaticFileHandler::handle(request, response, mimeTypes_);
    }
}

} // namespace http
