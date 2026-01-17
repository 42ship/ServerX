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

HttpStatus mapStatError(int err) {
    if (err == ENOENT || err == ENOTDIR)
        return NOT_FOUND;
    if (err == EACCES)
        return FORBIDDEN;
    return INTERNAL_SERVER_ERROR;
}

HttpStatus mapDeleteError(int err) {
    if (err == EACCES || err == EPERM)
        return FORBIDDEN;
    return INTERNAL_SERVER_ERROR;
}

std::string getParentDir(const std::string &path) {
    std::string::size_type pos = path.rfind('/');
    if (pos == std::string::npos || pos == 0)
        return "/";
    return path.substr(0, pos);
}

HttpStatus tryDeletePath(const std::string &path, bool isDir) {
    if (!isDir)
        return unlink(path.c_str()) == 0 ? OK : mapDeleteError(errno);
    if (!isDirectoryEmpty(path))
        return CONFLICT;
    if (rmdir(path.c_str()) != 0)
        return mapDeleteError(errno);
    return OK;
}

} // anonymous namespace

void FileDeleteHandler::handle(Request const &req, Response &res) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);

    if (!req.location()->has("upload_path"))
        return (void)res.status(METHOD_NOT_ALLOWED);

    std::string path = req.resolvePath();

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) != 0)
        return (void)res.status(mapStatError(errno));

    if (access(getParentDir(path).c_str(), W_OK) != 0)
        return (void)res.status(FORBIDDEN);

    HttpStatus result = tryDeletePath(path, S_ISDIR(statbuf.st_mode));
    if (result != OK)
        return (void)res.status(result);

    res.status(NO_CONTENT);
    res.setNoBody();
}

} // namespace http
