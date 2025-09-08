#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"
#include "http/MimeTypes.hpp"
#include "http/error_pages.hpp"
#include "http/utils.hpp"
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>

namespace http {

StaticFileHandler::StaticFileHandler(MimeTypes const &mime) : mimeTypes_(mime) {
}

HttpResponse StaticFileHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                       config::LocationBlock const *l) const {
    if (!l || !s)
        return error_pages::generateErrorResponse(NOT_FOUND, req.version);
    std::string path = (l->root.empty() ? s->root : l->root) + (req.uri.substr(l->path.size() - 1));

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            return error_pages::generateErrorResponse(NOT_FOUND, req.version);
        } else if (errno == EACCES) {
            return error_pages::generateErrorResponse(FORBIDDEN, req.version);
        } else {
            return error_pages::generateErrorResponse(INTERNAL_SERVER_ERROR, req.version);
        }
    }
    if (S_ISDIR(statbuf.st_mode)) {
        std::string index_path;
        bool found_index = false;
        for (std::vector<std::string>::const_iterator it = l->index.begin(); it != l->index.end();
             ++it) {
            index_path = path + (path[path.size() - 1] == '/' ? "" : "/") + *it;
            if (access(index_path.c_str(), F_OK) == 0) {
                found_index = true;
                path = index_path;
                break;
            }
        }
        if (!found_index) {
            return error_pages::generateErrorResponse(NOT_FOUND, req.version);
        }
        if (stat(path.c_str(), &statbuf) != 0) {
            return error_pages::generateErrorResponse(INTERNAL_SERVER_ERROR, req.version);
        }
    }
    if (!(statbuf.st_mode & S_IRUSR))
        return error_pages::generateErrorResponse(FORBIDDEN, req.version);
    // TODO: Add Body in memory if the file is small
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        return error_pages::generateErrorResponse(INTERNAL_SERVER_ERROR, req.version);
    }
    HttpResponse res(OK, req.version);
    res.setBodyFromFile(fd, statbuf.st_size);
    res.getHeaders()["Content-Type"] = this->mimeTypes_.getMimeType(utils::getFileExtension(path));
    return res;
}

} // namespace http
