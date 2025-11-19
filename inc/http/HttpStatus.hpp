#pragma once

#include <string>

namespace http {

/**
 * @enum Status
 * @brief Enumeration of common HTTP status codes.
 */
enum HttpStatus {
    UNKNOWN_STATUS = 0,
    OK = 200,         /** 200 OK — request succeeded. */
    CREATED = 201,    /** 201 Created — resource created successfully. */
    ACCEPTED = 202,   /** 202 Accepted — request accepted for processing, but not yet completed. */
    NO_CONTENT = 204, /** 204 No Content — request succeeded, no response body. */

    BAD_REQUEST = 400,  /** 400 Bad Request — malformed request syntax or invalid parameters. */
    UNAUTHORIZED = 401, /** 401 Unauthorized — authentication required or has failed. */
    FORBIDDEN = 403, /** 403 Forbidden — client is authenticated but not permitted to perform this
                        action. */
    NOT_FOUND = 404, /** 404 Not Found — requested resource does not exist. */
    METHOD_NOT_ALLOWED =
        405,        /** 405 Method Not Allowed — HTTP method not permitted for this resource. */
    CONFLICT = 409, /** 409 Conflict — request could not be completed due to a resource conflict
                       (e.g., non-empty directory on DELETE). */
    LENGTH_REQUIRED = 411,   /** 411 Length Required — missing Content-Length header for a request
                                that requires it. */
    PAYLOAD_TOO_LARGE = 413, /** 413 Payload Too Large — request body exceeds configured limit. */
    UNSUPPORTED_MEDIA_TYPE = 415, /** 415 Unsupported Media Type — request entity format is not
                                     supported by the server. */

    INTERNAL_SERVER_ERROR = 500, /** 500 Internal Server Error — generic server-side failure. */
    NOT_IMPLEMENTED = 501, /** 501 Not Implemented — server does not support the functionality
                                    required to fulfill the request. */
    BAD_GATEWAY = 502,
};

HttpStatus toHttpStatus(int);

HttpStatus toHttpStatus(const std::string &codeStr);

const char *getReasonPhrase(HttpStatus status);

} // namespace http
