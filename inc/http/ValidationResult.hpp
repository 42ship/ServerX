#pragma once

#include "http/HttpStatus.hpp"
#include <string>

namespace utils {

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

} // namespace utils
