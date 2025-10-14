#include "http/Handler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/MimeTypes.hpp"
#include "http/error_pages.hpp"
#include "http/utils.hpp"
#include <cstddef>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

namespace details {

std::string getPath(HttpRequest const &req, config::LocationBlock const &l) {
    std::string path;

    if (l.path() == req.path && req.path[req.path.length() - 1] != '/')
        path = req.path;
    else
        path = (req.path.substr(l.path().size() - 1));
    return l.path() + path;
}
} // namespace details

StaticFileHandler::StaticFileHandler(MimeTypes const &mime) : mimeTypes_(mime) {}

HttpResponse StaticFileHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                       config::LocationBlock const *l) const {
    if (!l || !s)
        return error_pages::generateErrorResponse(NOT_FOUND, req.version);
    std::string path = details::getPath(req, *l);
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
        config::StringVector const &indexes = l->indexFiles();
        for (size_t i = 0; i < indexes.size(); i++) {
            index_path = path + (path[path.size() - 1] == '/' ? "" : "/") + indexes[i];
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
