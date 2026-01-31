#include "http/Handler.hpp"

#include "common/filesystem.hpp"
#include "common/string.hpp"
#include "config/ServerBlock.hpp"
#include "http/FileUploadValidator.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "utils/Logger.hpp"

#include <cerrno>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace http {

namespace {
std::string getUploadPath(Request const &req) {
    if (req.location()->has("upload_path")) {
        return req.location()->getFirstRawValue("upload_path");
    }
    return "";
}

} // namespace

void FileUploadHandler::handle(Request const &req, Response &res, MimeTypes const &mime) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    std::string uploadPath = getUploadPath(req);
    std::string lRoot = req.location()->root();

    LOG_DEBUG("Location root is: " + lRoot + "   uri path: " + req.location()->path());

    if (uploadPath.empty()) {
        return (void)res.status(METHOD_NOT_ALLOWED, "Uploads are disabled for this location");
    }
    std::string path = utils::joinPaths(lRoot, uploadPath);
    upload::UploadValidationResult vup = upload::validateUploadPath(path);
    if (!vup.result) {
        LOG_ERROR("upload::validateUploadPath(path) reported an error with path: " + path + ": " +
                  vup.message);
        return (void)res.status(vup.status, vup.message);
    }

    if (!req.body()) {
        return (void)res.status(BAD_REQUEST, "No body provided for upload");
    }

    if (!req.headers().has("Content-Length")) {
        return (void)res.status(LENGTH_REQUIRED, "Content-Length header is required for uploads");
    }

    // todo: Handle multipart/form-data
    if (req.headers().has("Content-Type")) {
        std::string contentType = req.headers().get("Content-Type");
        if (contentType.find("multipart/form-data") != std::string::npos) {
            return (void)res.status(NOT_IMPLEMENTED);
        }
    }

    upload::UploadValidationResult pf = upload::parseFilename(req, mime);
    if (!pf.result) {
        LOG_DEBUG("upload::parseFilename(req, mime) reported an error");
        return (void)res.status(pf.status, pf.message);
    }

    if (!path.empty() && path[path.size() - 1] != '/') {
        path += '/';
    }
    path += pf.filename;

    LOG_SDEBUG("UPLOAD PATH: " + path);
    if (access(path.c_str(), F_OK) == 0) {
        return (void)res.status(CONFLICT, "File already exists");
    }
    utils::TempFile::MoveStatus moveRes = req.moveBody(path);
    if (moveRes != utils::TempFile::MOVE_SUCCESS) {
        LOG_SERROR("req.moveBody(path) reported error status: " << moveRes);
        return (void)res.status(INTERNAL_SERVER_ERROR, "Failed to write uploaded file to disk");
    }

    res.status(CREATED);

    // Build correct Location URL
    std::string lp = req.location()->path(); // e.g. "/img/"
    std::string up = getUploadPath(req);     // e.g. "upload" or "/upload"

    // Normalize: remove leading/trailing slashes for a robust prefix compare
    std::string toFind = lp.empty() ? lp : (lp[0] == '/' ? lp.substr(1) : lp);
    if (!toFind.empty() && toFind[toFind.size() - 1] == '/')
        toFind.resize(toFind.size() - 1);

    std::string upNorm = up.empty() ? up : (up[0] == '/' ? up.substr(1) : up);

    // TODO(root):
    // This logic exists because root & location path are merged too early.
    // Remove after root/url separation is implemented.
    if (!toFind.empty() && upNorm.compare(0, toFind.size(), toFind) == 0 &&
        (lRoot == req.server()->root())) {
        upNorm.erase(0, toFind.size());
        // remove an accidental leading slash if present after erase
        if (!upNorm.empty() && upNorm[0] == '/')
            upNorm.erase(0, 1);
    }

    std::string url = utils::joinPaths(lp, utils::joinPaths(upNorm, pf.filename));

    res.headers().add("Location", url);
}
} // namespace http