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

class CGIHandler : public IHandler {
public:
    void handle(Request const &, Response &) const;
};

class JsonErrorHandler {
public:
    static void populateResponse(Response &res);
private:
    JsonErrorHandler();
};

class DefaultErrorHandler {
public:
    DefaultErrorHandler(MimeTypes const &);
    void handle(Request const &, Response &) const;
    static void populateResponse(Response &res);
private:
    DefaultErrorHandler();
    void serveErrorFile(Response &response, const std::string &root, std::string fpath) const;
    MimeTypes const &mimeTypes_;
};

#define CHECK_FOR_SERVER_AND_LOCATION(req, res)                                                    \
    if (!req.location() || !req.server())                                                          \
    return (void)res.status(NOT_FOUND)

} // namespace http
