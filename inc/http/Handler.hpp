#pragma once

#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

namespace http {

/**
 * @class IHandler
 * @brief Abstract interface for all request handlers (Strategy Pattern).
 *
 * Defines the contract for stateless handler objects that process HTTP requests.
 */
class IHandler {
public:
    virtual ~IHandler() {};
};

/**
 * @brief Handles serving static files from the filesystem.
 */
class StaticFileHandler : public IHandler {
public:
    static void handle(Request const &, Response &, MimeTypes const &);
};

class CGIHandler : public IHandler {
public:
    static void handle(Request const &, Response &);
};

class JsonErrorHandler {
public:
    static void populateResponse(Response &);
};

class DefaultErrorHandler {
public:
    static void handle(Request const &request, Response &response, MimeTypes const &mimeTypes);
    static void populateResponse(Response &);
};

#define CHECK_FOR_SERVER_AND_LOCATION(req, res)                                                    \
    if (!req.location() || !req.server())                                                          \
    return (void)res.status(NOT_FOUND)

} // namespace http
