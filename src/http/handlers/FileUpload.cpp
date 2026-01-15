#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

namespace {

std::string extractFilename(Headers const &headers) {
    if (headers.has("X-Filename"))
        return headers.get("X-Filename");
    if (!headers.has("Content-Disposition"))
        return "";
    std::string disposition = headers.get("Content-Disposition");
    std::string::size_type pos = disposition.find("filename=");
    if (pos == std::string::npos)
        return "";
    return disposition.substr(pos + 9);
}

std::string resolveUploadDir(Request const &req) {
    std::string uploadPath = req.location()->getFirstRawValue("upload_path");
    if (uploadPath.empty())
        return "";
    if (uploadPath[0] == '/')
        return uploadPath;
    std::string root = req.location()->root();
    if (root.empty())
        root = req.server()->root();
    if (!root.empty() && root[root.size() - 1] != '/')
        root += '/';
    return root + uploadPath;
}

HttpStatus copyBodyToFile(int srcFd, std::string const &destPath) {
    int destFd = open(destPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destFd < 0)
        return INTERNAL_SERVER_ERROR;

    if (lseek(srcFd, 0, SEEK_SET) < 0) {
        close(destFd);
        return INTERNAL_SERVER_ERROR;
    }

    char buf[4096];
    ssize_t bytesRead;
    while ((bytesRead = read(srcFd, buf, sizeof(buf))) > 0) {
        ssize_t written = 0;
        while (written < bytesRead) {
            ssize_t w = write(destFd, buf + written, bytesRead - written);
            if (w < 0) {
                close(destFd);
                unlink(destPath.c_str());
                return INTERNAL_SERVER_ERROR;
            }
            written += w;
        }
    }

    close(destFd);
    if (bytesRead < 0) {
        unlink(destPath.c_str());
        return INTERNAL_SERVER_ERROR;
    }
    return OK;
}

} // anonymous namespace

void FileUploadHandler::handle(Request const &req, Response &res) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);

    if (!req.location()->has("upload_path"))
        return (void)res.status(METHOD_NOT_ALLOWED);

    std::string uploadDir = resolveUploadDir(req);
    if (uploadDir.empty())
        return (void)res.status(INTERNAL_SERVER_ERROR);

    struct stat statbuf;
    if (stat(uploadDir.c_str(), &statbuf) != 0 || !S_ISDIR(statbuf.st_mode))
        return (void)res.status(INTERNAL_SERVER_ERROR);

    if (access(uploadDir.c_str(), W_OK) != 0)
        return (void)res.status(FORBIDDEN);

    std::string filename = extractFilename(req.headers());
    if (filename.empty())
        return (void)res.status(BAD_REQUEST);

    if (uploadDir[uploadDir.size() - 1] != '/')
        uploadDir += '/';
    std::string destPath = uploadDir + filename;

    HttpStatus result = copyBodyToFile(req.body(), destPath);
    if (result != OK)
        return (void)res.status(result);

    std::string location = req.location()->path();
    std::string uploadPath = req.location()->getFirstRawValue("upload_path");
    if (!uploadPath.empty() && uploadPath[0] != '/') {
        if (location[location.size() - 1] != '/')
            location += '/';
        location += uploadPath;
    }
    if (location[location.size() - 1] != '/')
        location += '/';
    location += filename;

    res.status(CREATED);
    res.headers().add("Location", location);
    res.setNoBody();
}

} // namespace http
