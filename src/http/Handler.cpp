#include "http/Handler.hpp"
#include "http/error_pages.hpp"

#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <errno.h>
#include <dirent.h>

namespace http {

namespace {
Status errnoErrorHandling() {
    if (errno == ENOENT) {
        return NOT_FOUND;
    }
    if (errno == EACCES) {
        return FORBIDDEN;
    }
    return INTERNAL_SERVER_ERROR;
}

Response handleError(HttpRequest const &req, MimeTypes *mime) {
    Status code = errnoErrorHandling();
    if (access(defaultErrorFile, R_OK) == 0) {
        return Response(req.version, req.headers, StatusCode(code),
                        ResponseContent(defaultErrorFile, mime));
    }
    switch (code) {
    case NOT_FOUND:
        return error_pages::notFound(req.version, req.headers);
    case FORBIDDEN:
        return error_pages::forbidden(req.version, req.headers);
    default:
        return error_pages::internalServerError(req.version, req.headers);
    }
}

Response buildRawResponse(HttpRequest const &req, config::ServerBlock const *s,
                          config::LocationBlock const *l, MimeTypes *mime,
                          const std::string &fileName) {
    (void)s;
    if (!fileName.empty()) {
        errno = 0;
        std::string path = l->root + l->path + fileName;
        if (access(path.c_str(), R_OK) == 0) {
            return Response(req.version, req.headers, StatusCode(OK),
                            ResponseContent(path.c_str(), mime));
        }
        return handleError(req, mime);
    }
    return error_pages::notFound(req.version, req.headers);
}

} // namespace

Response IHandler::handle(HttpRequest const &h, RouterResult const &r, MimeTypes *mime) const {
    return handle(h, r.server, r.location, mime);
}

Response StaticFileHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                   config::LocationBlock const *l, MimeTypes *mime) const {
    if (!l || !s) {
        return NotFoundHandler().handle(req, s, l, mime);
    }
    std::string fileName = req.uri.substr(req.uri.find_last_of("/") + 1);

    if (fileName.empty()) {
        if (l->index.empty()) {
            fileName = "index.html";
        } else {
            for (std::vector<std::string>::const_iterator it = l->index.begin();
                 it != l->index.end(); it++) {
                fileName = *it;
                std::string tmpPath = l->root + l->path + fileName;
                if (access(tmpPath.c_str(), R_OK) == 0)
                    break;
            }
        }
        return buildRawResponse(req, s, l, mime, fileName);
    }
    if (!l->index.empty()) {
        std::vector<std::string>::const_iterator it =
            std::find(l->index.begin(), l->index.end(), fileName);
        fileName.clear();
        if (it != l->index.end()) {
            fileName = *it;
        }
    }
    return buildRawResponse(req, s, l, mime, fileName);
}

Response NotFoundHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                 config::LocationBlock const *l, MimeTypes *mime) const {
    (void)s;
    errno = 0;
    if (l && l->path == "/error_page/") {
        std::string path = l->root + l->path + "custom_404.html";
        if (access(path.c_str(), R_OK) == 0) {
            return Response(req.version, req.headers, StatusCode(NOT_FOUND),
                            ResponseContent(path.c_str(), mime));
        }
    }
    return handleError(req, mime);
}

Response CGIHandler::handle(HttpRequest const &, config::ServerBlock const *s,
                            config::LocationBlock const *l, MimeTypes *mime) const {
    (void)s;
    (void)l;
    (void)mime;
    return Response();
}

Response ErrorHandler::handle(HttpRequest const &, config::ServerBlock const *s,
                              config::LocationBlock const *l, MimeTypes *mime) const {
    (void)s;
    (void)l;
    (void)mime;
    return Response();
}
} // namespace http