#include "http/utils.hpp"
#include "common/filesystem.hpp"
#include "common/string.hpp"
#include "config/Block.hpp"
#include "http/HttpRequest.hpp"
#include "http/HttpStatus.hpp"
#include "http/MimeTypes.hpp"
#include "utils/Logger.hpp"
#include <errno.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace utils {

std::string extractHeaderParam(const std::string &str, const std::string &toFind) {
    std::istringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ';')) {
        token = trim(token);
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

ValidationResult parseFilename(http::HttpRequest const &req, http::MimeTypes const &mime) {
    std::string filename = req.getHeader("X-Filename");
    std::string disposition = req.getHeader("Content-Disposition");
    if (filename.empty()) {
        if (disposition.empty()) {
            return ValidationResult::fail(http::BAD_REQUEST,
                                          "Missing filename (X-Filename or Content-Disposition)");
        } else {
            filename = extractHeaderParam(disposition, "filename=");
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
    if (contentLength.empty()){
        return utils::ValidationResult::fail(http::LENGTH_REQUIRED, "Content-Length header is required for uploads");
    }

    if (!s.has("upload_file_size")) {
        return ValidationResult::ok("");
    }
    const config::StringVector &sv = s.get("upload_file_size");
    if (sv.empty()) {
        return ValidationResult::ok("");
    }
    std::string sUploadFileSize = sv[0];
    size_t uploadFileSize = fromString<size_t>(sUploadFileSize);
    size_t len = fromString<size_t>(contentLength);
    uploadFileSize = uploadFileSize * 1024 * 1024;
    if (len > uploadFileSize) {
        return utils::ValidationResult::fail(http::PAYLOAD_TOO_LARGE,
                                             "Payload exceeds configured upload_file_size");
    }
    return utils::ValidationResult::ok("");
}

ValidationResult checkContentLength(std::string const &contentLen)
{
    if (contentLen.empty()){
        return utils::ValidationResult::fail(http::LENGTH_REQUIRED, "Content-Length header is required for uploads");
    }
    return utils::ValidationResult::ok("");
}

} // namespace utils
