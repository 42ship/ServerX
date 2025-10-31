#include "common/filesystem.hpp"
#include "http/Handler.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/MimeTypes.hpp"
#include <cstddef>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

StaticFileHandler::StaticFileHandler(MimeTypes const &mime) : mimeTypes_(mime) {}

void StaticFileHandler::handle(HttpRequest const &req, HttpResponse &res) {
    (void)req;
    // if (!l || !s)
    //     return error_pages::generateErrorResponse(NOT_FOUND, req.version);
    // std::string path = utils::getPath(req, *l);
    std::string path = "index.html";
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        return;
        // if (errno == ENOENT || errno == ENOTDIR) {
        //     return error_pages::generateErrorResponse(NOT_FOUND, req.version);
        // } else if (errno == EACCES) {
        //     return error_pages::generateErrorResponse(FORBIDDEN, req.version);
        // } else {
        //     return error_pages::generateErrorResponse(INTERNAL_SERVER_ERROR, req.version);
        // }
    }
    if (S_ISDIR(statbuf.st_mode)) {
        return;
#if 0
        if (!req.location->has("index"))
            return error_pages::generateErrorResponse(NOT_FOUND, req.version);
        std::string index_path;
        bool found_index = false;
        std::vector<std::string> const &indexes = l->indexFiles();
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
#endif
    }
    if (!(statbuf.st_mode & S_IRUSR))
        return;
    // return error_pages::generateErrorResponse(FORBIDDEN, req.version);
    res.startLine.statusCode = OK;
    res.startLine.protocol = "HTTP/1.1";
    res.setBodyFromFile(path, mimeTypes_.getMimeType(utils::getFileExtension(path)));
}

} // namespace http
