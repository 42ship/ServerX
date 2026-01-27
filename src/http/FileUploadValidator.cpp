#include "http/FileUploadValidator.hpp"

#include "common/filesystem.hpp"
#include "common/string.hpp"

#include <sstream>
#include <unistd.h>
#include <cerrno>

namespace http::upload {

UploadValidationResult UploadValidationResult::ok(const std::string &file) {
    UploadValidationResult res;
    res.filename = file;
    res.status = OK;
    res.result = true;
    return res;
}

UploadValidationResult UploadValidationResult::ok() { return ok(""); }

UploadValidationResult UploadValidationResult::fail(HttpStatus s, const std::string &msg) {
    UploadValidationResult res;
    res.status = s;
    res.message = msg;
    res.result = false;
    return res;
}

std::string extractHeaderParam(const std::string &str, const std::string &toFind) {
    std::istringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ';')) {
        token = utils::trim(token);
        std::string::size_type pos = token.find(toFind);
        if (pos != std::string::npos) {
            std::string line = token.substr(pos + toFind.length());
            if (!line.empty() && line[0] == '"') {
                line.erase(0, 1);
            }
            if (!line.empty() && line[line.size() - 1] == '"') {
                line.erase(line.size() - 1);
            }
            return line;
        }
    }
    return "";
}

UploadValidationResult parseFilename(Request const &req, MimeTypes const &mime) {
    std::string filename = req.headers().get("X-Filename");
    std::string disposition = req.headers().get("Content-Disposition");

    if (!filename.empty() && !disposition.empty()) {
        return UploadValidationResult::fail(
            BAD_REQUEST,
            "Conflicting filename sources: both X-Filename and Content-Disposition provided");
    }
    if (filename.empty()) {
        if (disposition.empty()) {
            return UploadValidationResult::fail(
                BAD_REQUEST, "Missing filename (X-Filename or Content-Disposition)");
        } else {
            filename = extractHeaderParam(disposition, "filename=");
        }
    }
    if (filename.empty()) {
        return UploadValidationResult::fail(BAD_REQUEST, "Empty filename in Content-Disposition");
    }

    std::string::size_type index = filename.rfind('.');
    if (index != std::string::npos && index == filename.size() - 1) {
        return UploadValidationResult::fail(BAD_REQUEST, "Invalid filename (trailing dot)");
    }

    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos ||
        filename.find("..") != std::string::npos) {
        return UploadValidationResult::fail(BAD_REQUEST,
                                            "Invalid filename: directory traversal attempt");
    }

    std::string contentType = req.headers().get("Content-Type");
    if (contentType.empty()) {
        return UploadValidationResult::fail(BAD_REQUEST, "Missing Content-Type");
    }

    if (contentType == "application/octet-stream") {
        return UploadValidationResult::ok(filename);
    }

    if (contentType.find("multipart/form-data") != std::string::npos) {
        return UploadValidationResult::fail(NOT_IMPLEMENTED, "multipart/form-data not implemented");
    }

    std::string ext;
    if (index != std::string::npos) {
        ext = filename.substr(index + 1);
    }
    std::string mimeType = mime.getMimeType(ext);
    if (contentType != mimeType) {
        return UploadValidationResult::fail(UNSUPPORTED_MEDIA_TYPE,
                                            "Mismatched Content-Type and file extension");
    }
    return UploadValidationResult::ok(filename);
}

UploadValidationResult validateUploadPath(const std::string &path) {
    if (path.empty()) {
        return UploadValidationResult::fail(METHOD_NOT_ALLOWED,
                                            "Uploads are disabled for this location");
    }

    if (!utils::isDir(path)) {
        return UploadValidationResult::fail(
            INTERNAL_SERVER_ERROR, "Upload directory is missing or not a directory: " + path);
    }

    if (access(path.c_str(), W_OK | X_OK) == -1) {
        if (errno == EACCES || errno == EROFS) {
            return UploadValidationResult::fail(FORBIDDEN,
                                                "Upload directory is not writable: " + path);
        }
        return UploadValidationResult::fail(INTERNAL_SERVER_ERROR,
                                            "Failed to access upload directory: " + path);
    }

    return UploadValidationResult::ok();
}

} // namespace http::upload