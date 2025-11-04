#pragma once

#include "http/Request.hpp"
#include "http/Response.hpp"
#include "http/MimeTypes.hpp"

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

class JsonErrorHandler : public IHandler {
public:
    static void populateResponse(Response &res);
};

class DefaultErrorHandler : public IHandler {
public:
    static void populateResponse(Response &res);
};

#if 0

/**
 * @brief Handles the generation of not found(404).
 */
class NotFoundHandler : public IHandler {
public:
    Response handle(Request const &req, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL) const;
};

/**
 * @brief Handles the execution of CGI scripts.
 */
class CGIHandler : public IHandler {
    Response handle(Request const &, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL) const;
};

/**
 * @brief Handles the execution of CGI scripts.
 */
class DefaultErrorHandler : public IHandler {
    Response handle(Request const &, config::ServerBlock const *s = NULL,
                        config::LocationBlock const *l = NULL) const;
};

class FileUploadHandler : public IHandler {
public:
    FileUploadHandler(MimeTypes const &);
    Response handle(Request const &req, config::ServerBlock const *s,
                        config::LocationBlock const *l) const;

private:
    FileUploadHandler();
    Response handleMultipartFormData(Request const &req, config::ServerBlock const *s,
                                         config::LocationBlock const *l) const;
    MimeTypes const &mimeTypes_;
};

class FileDeleteHandler : public IHandler {
public:
    Response handle(Request const &req, config::ServerBlock const *s,
                        config::LocationBlock const *l) const;
};

namespace details {
Request parse(std::istringstream &s, const std::string &boundary);
}
#endif

} // namespace http
