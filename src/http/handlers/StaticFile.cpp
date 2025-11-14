#include "common/filesystem.hpp"
#include "config/ServerBlock.hpp"
#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include <cerrno>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

StaticFileHandler::StaticFileHandler(MimeTypes const &mime) : mimeTypes_(mime) {}

void StaticFileHandler::handle(Request const &req, Response &res) const {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    std::string path = req.resolvePath();
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0) {
        if (errno == ENOENT || errno == ENOTDIR) {
            return (void)res.status(NOT_FOUND);
        } else if (errno == EACCES) {
            return (void)res.status(FORBIDDEN);
        } else {
            return (void)res.status(INTERNAL_SERVER_ERROR);
        }
    }
    if (S_ISDIR(statbuf.st_mode)) {
        if (!req.location()->has("index"))
            return (void)res.status(NOT_FOUND);
        std::string index_path;
        bool found_index = false;
        std::vector<std::string> const &indexes = req.location()->indexFiles();
        for (size_t i = 0; i < indexes.size(); i++) {
            index_path = path + (path[path.size() - 1] == '/' ? "" : "/") + indexes[i];
            if (access(index_path.c_str(), F_OK) == 0) {
                found_index = true;
                path = index_path;
                break;
            }
        }
        if (!found_index) {
            return (void)res.status(NOT_FOUND);
        }
        if (stat(path.c_str(), &statbuf) != 0) {
            return (void)res.status(INTERNAL_SERVER_ERROR);
        }
    }
    if (!(statbuf.st_mode & S_IRUSR))
        return (void)res.status(FORBIDDEN);
    res.status(OK);
    try {
        res.setBodyFromFile(path, mimeTypes_.getMimeType(utils::getFileExtension(path)));
    } catch (...) {
        res.status(INTERNAL_SERVER_ERROR);
    }
}

} // namespace http
