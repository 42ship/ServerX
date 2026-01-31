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

void StaticFileHandler::handle(Request const &req, Response &res, MimeTypes const &mime) {
    if (!req.server() || !req.location()) {
        res.status(INTERNAL_SERVER_ERROR);
        return;
    }

    std::string path = req.resolvePath();
    struct stat st;

    if (!utils::getFileStatus(path, st)) {
        if (errno == EACCES) {
            res.status(FORBIDDEN);
        } else {
            res.status(NOT_FOUND);
        }
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (req.path().empty() || req.path()[req.path().size() - 1] != '/') {
            std::string redirect = req.path() + "/";
            if (!req.queryString().empty())
                redirect += "?" + req.queryString();
            res.status(MOVED_PERMANENTLY);
            res.headers().add("Location", redirect);
            return;
        }

        std::string indexPath = req.location()->resolveIndexFile(path);

        if (indexPath.empty()) {
            res.status(NOT_FOUND);
            return;
        }
        path = indexPath;
    }

    if (access(path.c_str(), R_OK) != 0) {
        res.status(FORBIDDEN);
        return;
    }

    res.status(OK);
    try {
        std::string ext = utils::getFileExtension(path);
        res.setBodyFromFile(path, mime.getMimeType(ext));
    } catch (...) {
        res.status(INTERNAL_SERVER_ERROR);
    }
}
} // namespace http
