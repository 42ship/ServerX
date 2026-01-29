#include "http/FileUploadValidator.hpp"

#include "common/filesystem.hpp"
#include "common/string.hpp"

#include <cerrno>
#include <sstream>
#include <unistd.h>

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

UploadValidationResult parseFilename(Request const &req, MimeTypes const &) {
    std::string filename = req.headers().get("X-Filename");
    std::string disposition = req.headers().get("Content-Disposition");

    // 1. Source filename from headers if present
    if (filename.empty() && !disposition.empty()) {
        filename = extractHeaderParam(disposition, "filename=");
    }

    // 2. Fallback: Source filename from the URI path itself
    // e.g. POST /uploads/my_file.txt -> "my_file.txt"
    if (filename.empty()) {
        std::string const &reqPath = req.path();
        size_t lastSlash = reqPath.find_last_of('/');
        if (lastSlash != std::string::npos && lastSlash < reqPath.size() - 1) {
            filename = reqPath.substr(lastSlash + 1);
        }
    }

    if (filename.empty()) {
        return UploadValidationResult::fail(
            BAD_REQUEST, "Missing filename (X-Filename, Content-Disposition, or URI path)");
    }

    // 3. Security & Validity Sanitization
    // Prevent directory traversal
    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos ||
        filename.find("..") != std::string::npos) {
        return UploadValidationResult::fail(BAD_REQUEST,
                                            "Invalid filename: directory traversal attempt");
    }

    // Prevent hidden files (optional, but safer) or trailing dots
    if (filename[0] == '.') {
        return UploadValidationResult::fail(BAD_REQUEST, "Invalid filename: cannot start with dot");
    }

    size_t dotPos = filename.rfind('.');
    if (dotPos != std::string::npos && dotPos == filename.size() - 1) {
        return UploadValidationResult::fail(BAD_REQUEST, "Invalid filename: trailing dot");
    }

    // 4. Content-Type check (Advisory)
    // We don't strictly enforce mime-match here anymore as it prevents valid raw uploads
    // from tools that don't know the mime type (like simple curl).
    std::string contentType = req.headers().get("Content-Type");
    if (contentType.find("multipart/form-data") != std::string::npos) {
        return UploadValidationResult::fail(NOT_IMPLEMENTED,
                                            "multipart/form-data not yet supported");
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