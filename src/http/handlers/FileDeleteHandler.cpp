#include "http/Handler.hpp"

#include <cstddef>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "http/error_pages.hpp"
#include "http/utils.hpp"

namespace http {

static std::string parentDir(const std::string &p) {
    std::string::size_type pos = p.find_last_of('/');
    if (pos == std::string::npos)
        return ".";
    if (pos == 0)
        return "/";
    return p.substr(0, pos);
}

HttpResponse FileDeleteHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                       config::LocationBlock const *l) const {
    (void)s;
    if (!l) {
        return error_pages::generateJsonErrorResponse(NOT_FOUND, req.version,
                                                      "No matching location for path: " + req.path);
    }
    std::string path = utils::getPath(req, *l);
    if (path.empty()) {
        return error_pages::generateJsonErrorResponse(
            NOT_FOUND, req.version, "Requested resource not found or invalid path");
    }

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            return error_pages::generateJsonErrorResponse(
                NOT_FOUND, req.version, "Requested resource not found or invalid path");
        } else if (errno == EACCES) {
            return error_pages::generateJsonErrorResponse(FORBIDDEN, req.version,
                                                          "Permission denied");
        } else {
            return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version,
                                                          "Failed to stat resource");
        }
    }

    // check permissions of parent directory
    if (access(parentDir(path).c_str(), W_OK | X_OK) == -1) {
        if (errno == EACCES || errno == EROFS) {
            return error_pages::generateJsonErrorResponse(FORBIDDEN, req.version,
                                                          "Permission denied");
        }
        return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version,
                                                      "Access check failed");
    }

    if (remove(path.c_str()) != 0) {
        switch (errno) {
        case ENOENT:
        case ENOTDIR:
            return error_pages::generateJsonErrorResponse(NOT_FOUND, req.version,
                                                          "Resource not found");
        case EACCES:
        case EROFS:
        case EPERM:
            return error_pages::generateJsonErrorResponse(FORBIDDEN, req.version,
                                                          "Permission denied");
        case ENOTEMPTY:
        case EEXIST:
            return error_pages::generateJsonErrorResponse(CONFLICT, req.version,
                                                          "Directory is not empty");
        default:
            return error_pages::generateJsonErrorResponse(INTERNAL_SERVER_ERROR, req.version,
                                                          "Failed to remove resource");
        }
    }

    HttpResponse res(NO_CONTENT, req.version);
    res.setNoBody();
    return res;
}

} // namespace http
