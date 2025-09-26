#pragma once

namespace http {

/**
 * @enum Status
 * @brief Enumeration of common HTTP status codes.
 */
enum Status {
    OK = 200,         /** 200 OK — request succeeded. */
    CREATED = 201,    /** 201 Created — resource created. */
    ACCEPTED = 202,   /** 202 Accepted — processing pending. */
    NO_CONTENT = 204, /** 204 No Content — successful, no body. */

    BAD_REQUEST = 400,              /** 400 Bad Request — malformed syntax. */
    UNAUTHORIZED = 401,             /** 401 Unauthorized — authentication required/failed. */
    FORBIDDEN = 403,                /** 403 Forbidden — not allowed despite authentication. */
    NOT_FOUND = 404,                /** 404 Not Found — resource not found. */
    METHOD_NOT_ALLOWED = 405,       /** 405 Method Not Allowed — method not permitted. */
    PAYLOAD_TOO_LARGE = 413,        /** 413 Payload Too Large — file size exceeds the allowed limit*/
    UNSUPPORTED_MEDIA_TYPE = 415,   /** 415 Unsupported Media Type — unsupported file type*/

    INTERNAL_SERVER_ERROR = 500 /** 500 Internal Server Error — server-side failure. */
};

} // namespace http
