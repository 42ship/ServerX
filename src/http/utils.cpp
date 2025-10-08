#include "http/utils.hpp"
#include "common/filesystem.hpp"
#include "common/string.hpp"
#include "config/internal/Block.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpStatus.hpp"
#include "http/MimeTypes.hpp"
#include <errno.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace utils {

static std::string extractFilename(const std::string &disposition) {
    std::istringstream ss(disposition);
    std::string token;

    while (std::getline(ss, token, ';')) {
        token = trim(token);
        std::string::size_type pos = token.find("filename=");
        if (pos != std::string::npos) {
            std::string filename = token.substr(pos + 9); // 9 = strlen("filename=")
            if (!filename.empty() && filename[0] == '"') {
                filename.erase(0, 1);
            }
            if (!filename.empty() && filename[filename.size() - 1] == '"') {
                filename.erase(filename.size() - 1);
            }
            return filename;
        }
    }
    return "";
}

ValidationResult parseFilename(http::HttpRequest const &req, http::MimeTypes const &mime) {
    std::string filename = req.getHeader("X-Filename");
    std::string disposition = req.getHeader("Content-Disposition");
    if (filename.empty()) {
        if (disposition.empty()) {
            return ValidationResult::fail(http::BAD_REQUEST,
                                          "Missing filename (X-Filename or Content-Disposition)");
        } else {
            filename = extractFilename(disposition);
        }
    }
    if (filename.empty()) {
        return ValidationResult::fail(http::BAD_REQUEST, "Empty filename in Content-Disposition");
    }
    size_t index = filename.find_last_of(".");
    if (index != std::string::npos && index == filename.size() - 1) {
        return ValidationResult::fail(http::BAD_REQUEST, "Invalid filename (trailing dot)");
    }

    std::string contentType = req.getHeader("Content-Type");
    if (contentType.empty()) {
        return ValidationResult::fail(http::BAD_REQUEST, "Missing Content-Type");
    }
    if (contentType == "application/octet-stream") {
        return ValidationResult::ok(filename);
    }

    if (contentType.find("multipart/form-data") != std::string::npos) {
        return ValidationResult::fail(http::UNSUPPORTED_MEDIA_TYPE,
                                      "multipart/form-data not supported");
    }
    std::string ext;
    if (index != std::string::npos) {
        ext = filename.substr(index + 1);
    }
    std::string mimeType = mime.getMimeType(ext);
    if (contentType != mimeType) {
        return ValidationResult::fail(http::UNSUPPORTED_MEDIA_TYPE,
                                      "Mismatched Content-Type and file extension");
    }
    return ValidationResult::ok(filename);
}

ValidationResult validateUploadPath(const std::string &path) {
    if (path.empty()) {
        return ValidationResult::fail(http::METHOD_NOT_ALLOWED,
                                      "Uploads are disabled for this location");
    }
    if (!isDir(path)) {
        return ValidationResult::fail(http::INTERNAL_SERVER_ERROR,
                                      "Upload directory is missing or not a directory: " + path);
    }
    if (access(path.c_str(), W_OK | X_OK) == -1) {
        if (errno == EACCES || errno == EROFS) {
            return ValidationResult::fail(http::FORBIDDEN,
                                          "Upload directory is not writable: " + path);
        }
        return ValidationResult::fail(http::INTERNAL_SERVER_ERROR,
                                      "Failed to access upload directory: " + path);
    }
    return ValidationResult::ok("");
}

/** todo: write own or chose better type for check limits */
ValidationResult checkUploadLimit(const std::string &contentLength, config::Block const &s) {
    const config::StringVector *sv = s.get("upload_file_size");
    if (!sv || sv->empty()) {
        return ValidationResult::ok("");
    }
    if (contentLength.empty()) {
        return ValidationResult::fail(http::LENGTH_REQUIRED,
                                      "Content-Length header is required for uploads");
    }
    std::string sUploadFileSize = (*sv)[0];
    size_t uploadFileSize = fromString<size_t>(sUploadFileSize);
    size_t len = fromString<size_t>(contentLength);
    uploadFileSize = uploadFileSize * 1024 * 1024;
    if (len > uploadFileSize) {
        return utils::ValidationResult::fail(http::PAYLOAD_TOO_LARGE,
                                             "Payload exceeds configured upload_file_size");
    }
    return utils::ValidationResult::ok("");
}

} // namespace utils
