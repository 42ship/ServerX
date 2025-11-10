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
    virtual void handle(Request const &, Response &) const = 0;
};

/**
 * @brief Handles serving static files from the filesystem.
 */
class StaticFileHandler : public IHandler {
public:
    StaticFileHandler(MimeTypes const &);
    void handle(Request const &, Response &) const;

private:
    StaticFileHandler();
    MimeTypes const &mimeTypes_;
};

class JsonErrorHandler {
public:
    static void populateResponse(Response &res);
};

class DefaultErrorHandler {
public:
    static void populateResponse(Response &res);
};

} // namespace http
