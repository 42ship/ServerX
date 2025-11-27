#pragma once

#include <string>

namespace http {

/**
 * @enum Status
 * @brief Enumeration of common HTTP status codes.
 */
enum HttpStatus {
    UNKNOWN_STATUS = 0,

    // 2xx Success
    OK = 200,         /** *Request succeeded. */
    CREATED = 201,    /** *Resource created. */
    ACCEPTED = 202,   /** Request accepted for processing. */
    NO_CONTENT = 204, /** *Request succeeded; no body in response (e.g., DELETE). */

    // 3xx Redirection
    MOVED_PERMANENTLY = 301, /** *Resource moved to new URL (permanent). */
    FOUND = 302,             /** *Resource temporarily at different URL. */
    SEE_OTHER = 303,         /** See another URL (often used after POST). */
    NOT_MODIFIED = 304,      /** Resource not modified since last access. */

    // 4xx Client Error
    BAD_REQUEST = 400,  /** *Malformed syntax or invalid parameters (client error). */
    UNAUTHORIZED = 401, /** *Authentication failed or is missing (Unauthenticated). */
    FORBIDDEN = 403,    /** *Client authenticated but not authorized for action (Unauthorized). */
    NOT_FOUND = 404,    /** *Requested resource doesn't exist. */
    METHOD_NOT_ALLOWED = 405,     /** *HTTP method not allowed for resource. */
    CONFLICT = 409,               /** *Request failed due to resource conflict (e.g., duplicate). */
    LENGTH_REQUIRED = 411,        /** Missing Content-Length header. */
    PAYLOAD_TOO_LARGE = 413,      /** Request body exceeds limit. */
    UNSUPPORTED_MEDIA_TYPE = 415, /** Request entity format is not supported. */

    // 5xx Server Error
    INTERNAL_SERVER_ERROR = 500, /** *Generic server-side failure. */
    NOT_IMPLEMENTED = 501,       /** Server lacks functionality to fulfill request. */
    BAD_GATEWAY = 502,           /** *Invalid response from upstream server (gateway/proxy). */
};

HttpStatus toHttpStatus(int);

HttpStatus toHttpStatus(const std::string &codeStr);

const char *getReasonPhrase(HttpStatus status);

} // namespace http
