#pragma once

#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "http/HttpRequest.hpp"
#include "Response.hpp"
#include "RouterResult.hpp"

namespace http {

#define defaultErrorFile "errorpage.html"

/**
 * @class IHandler
 * @brief Abstract interface for all request handlers (Strategy Pattern).
 *
 * Defines the contract for stateless handler objects that process HTTP requests.
 */
class IHandler {
public:
    virtual ~IHandler() {};
    /**
     * @brief The primary handler logic entry point.
     * @param request The client's HTTP request.
     * @param server The matched server configuration context.
     * @param location The matched location configuration context.
     * @return An HttpResponse object representing the result of the handling.
     */
    virtual Response handle(HttpRequest const &, config::ServerBlock const *,
                                config::LocationBlock const *, MimeTypes *) const = 0;

    /**
     * @brief Convenience overload to handle a request using a RouterResult.
     */
    Response handle(HttpRequest const &h, RouterResult const &r, MimeTypes *mime) const;
};

/**
 * @brief Handles serving static files from the filesystem.
 */
class StaticFileHandler : public IHandler {
public:
    Response handle(HttpRequest const &req, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL, MimeTypes *mime = NULL) const;
};

/**
 * @brief Handles the generation of not found(404).
 */
class NotFoundHandler : public IHandler {
public:
    Response handle(HttpRequest const &req, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL, MimeTypes* mime = NULL) const;
};

/**
 * @brief Handles the execution of CGI scripts.
 */
class CGIHandler : public IHandler {
    Response handle(HttpRequest const &, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL, MimeTypes *mime = NULL) const;
};

/**
 * @brief Handles the execution of CGI scripts.
 */
class ErrorHandler : public IHandler {
    Response handle(HttpRequest const &, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL, MimeTypes *mime = NULL) const;
};

} // namespace http
