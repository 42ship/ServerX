#include "config/LocationBlock.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include <cerrno>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

namespace {

bool isDirectoryEmpty(const std::string &path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        return false;
    }
    const struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' &&
            (entry->d_name[1] == '\0' ||
             (entry->d_name[1] == '.' && entry->d_name[2] == '\0'))) {
            continue;
        }
        closedir(dir);
        return false;
    }
    closedir(dir);
    return true;
}

} // anonymous namespace

void FileDeleteHandler::handle(Request const &req, Response &res) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);

    if (!req.location()->has("upload_path")) {
        return (void)res.status(METHOD_NOT_ALLOWED);
    }

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

    std::string parentDir = path.substr(0, path.rfind('/'));
    if (parentDir.empty()) {
        parentDir = "/";
    }
    if (access(parentDir.c_str(), W_OK) != 0) {
        return (void)res.status(FORBIDDEN);
    }

    if (S_ISDIR(statbuf.st_mode)) {
        if (!isDirectoryEmpty(path)) {
            return (void)res.status(CONFLICT);
        }
        if (rmdir(path.c_str()) != 0) {
            if (errno == EACCES || errno == EPERM) {
                return (void)res.status(FORBIDDEN);
            }
            return (void)res.status(INTERNAL_SERVER_ERROR);
        }
    } else {
        if (unlink(path.c_str()) != 0) {
            if (errno == EACCES || errno == EPERM) {
                return (void)res.status(FORBIDDEN);
            }
            return (void)res.status(INTERNAL_SERVER_ERROR);
        }
    }

    res.status(NO_CONTENT);
    res.setNoBody();
}

} // namespace http
