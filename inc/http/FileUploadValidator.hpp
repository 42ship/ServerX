#pragma once

#include "http/HttpStatus.hpp"

#include "http/MimeTypes.hpp"
#include "http/Request.hpp"

#include <string>

namespace http {

namespace upload {

/**
 * @brief Represents the result of a validation or parsing operation.
 *
 * This structure is used across upload-related utility functions to provide
 * both success and failure states in a consistent way.
 *
 * Fields:
 *  - `result`   — true if validation succeeded, false otherwise.
 *  - `filename` — extracted or validated filename in case of success.
 *  - `status`   — corresponding HTTP status code (e.g., 400, 403, 413, etc.).
 *  - `message`  — human-readable explanation in case of failure.
 *
 * Factory methods:
 *  - `ok(filename)`          — creates a successful result with HTTP 200 (OK)
 *                              and the provided filename.
 *  - `fail(status, message)` — creates a failed result with a specific error code
 *                              and message.
 */
struct UploadValidationResult {
    bool result;          ///< True if validation succeeded.
    std::string filename; ///< Parsed or validated filename (empty if failed).
    HttpStatus status;    ///< HTTP status code associated with the result.
    std::string message;  ///< Detailed error message if validation failed.

    /// @brief Creates a successful validation result with a filename.
    static UploadValidationResult ok(const std::string &file);

    /// @brief Creates a successful validation result with an empty filename.
    static UploadValidationResult ok();

    /// @brief Creates a failed validation result.
    static UploadValidationResult fail(HttpStatus s, const std::string &msg);
};

std::string extractHeaderParam(const std::string &str, const std::string &toFind);

UploadValidationResult parseFilename(Request const &req, MimeTypes const &mime);

UploadValidationResult validateUploadPath(const std::string &path);

} // namespace upload
} // namespace http