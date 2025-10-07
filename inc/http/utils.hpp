#pragma once

#include <errno.h>
#include <ostream>
#include <sys/stat.h>
#include <unistd.h>

#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "http/HttpStatus.hpp"

namespace http {
class HttpRequest; // forward decl
class MimeTypes;   // forward decl
} // namespace http

namespace utils {

enum HttpMethod { GET, POST, PUT, DELETE, UNKNOWN };

/**
 * @brief Represents the result of a validation or parsing operation.
 *
 * This structure is used across upload-related utility functions to provide
 * both success and failure states in a consistent way.
 *
 * Fields:
 *  - `result`  — true if validation succeeded, false otherwise.
 *  - `value`   — valid computed value (e.g., filename) if successful.
 *  - `status`  — corresponding HTTP status code (e.g., 400, 403, 413, etc.).
 *  - `message` — human-readable explanation in case of failure.
 *
 * Factory methods:
 *  - `ok(value)`   — creates a successful result with HTTP 200 (OK).
 *  - `fail(status, message)` — creates a failed result with a specific error.
 */
struct ValidationResult {
    bool result;         ///< True if validation succeeded.
    std::string value;   ///< Parsed value (e.g. filename) or empty if failed.
    http::Status status; ///< HTTP status code associated with the result.
    std::string message; ///< Detailed error message if validation failed.

    /// @brief Creates a successful validation result.
    static ValidationResult ok(const std::string &v) {
        ValidationResult vr;
        vr.status = http::OK;
        vr.value = v;
        vr.result = true;
        return vr;
    }

    /// @brief Creates a failed validation result.
    static ValidationResult fail(http::Status s, const std::string &msg) {
        ValidationResult vr;
        vr.status = s;
        vr.message = msg;
        vr.result = false;
        return vr;
    }
};

std::ostream &operator<<(std::ostream &o, HttpMethod);

HttpMethod matchHttpMethod(std::string const &);

std::string getFileExtension(std::string const &fpath);

inline bool isDir(const std::string &p) {
    struct stat st;
    return ::stat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

ValidationResult parseFilename(http::HttpRequest const &req, http::MimeTypes const &mime);

ValidationResult validateUploadPath(const std::string &path);

ValidationResult checkUploadLimit(const std::string &contentLength, config::ServerBlock const &s);

} // namespace utils
