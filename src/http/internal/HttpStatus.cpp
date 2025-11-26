#include "http/HttpStatus.hpp"
#include <cstdlib>
#include <string>

namespace http {

HttpStatus toHttpStatus(int code) {
    switch (code) {
        // clang-format off
        // 2xx Success
        case 200: return OK;
        case 201: return CREATED;
        case 202: return ACCEPTED;
        case 204: return NO_CONTENT;
        
        // 3xx Redirection (NEW)
        case 301: return MOVED_PERMANENTLY;
        case 302: return FOUND;
        case 303: return SEE_OTHER;
        case 304: return NOT_MODIFIED;

        // 4xx Client Error
        case 400: return BAD_REQUEST;
        case 401: return UNAUTHORIZED;
        case 403: return FORBIDDEN;
        case 404: return NOT_FOUND;
        case 405: return METHOD_NOT_ALLOWED;
        case 409: return CONFLICT;
        case 411: return LENGTH_REQUIRED;
        case 413: return PAYLOAD_TOO_LARGE;
        case 415: return UNSUPPORTED_MEDIA_TYPE;
        
        // 5xx Server Error
        case 500: return INTERNAL_SERVER_ERROR;
        case 501: return NOT_IMPLEMENTED;
        case 502: return BAD_GATEWAY;
        
        default: return UNKNOWN_STATUS;
        // clang-format on
    }
}

HttpStatus toHttpStatus(const std::string &codeStr) {
    int code = std::atoi(codeStr.c_str());
    return toHttpStatus(code);
}

const char *getReasonPhrase(HttpStatus status) {
    switch (status) {
        // clang-format off
        // 2xx Success
        case OK: return "OK";
        case CREATED: return "Created";
        case ACCEPTED: return "Accepted";
        case NO_CONTENT: return "No Content";

        // 3xx Redirection (NEW)
        case MOVED_PERMANENTLY: return "Moved Permanently";
        case FOUND: return "Found";
        case SEE_OTHER: return "See Other";
        case NOT_MODIFIED: return "Not Modified";

        // 4xx Client Error
        case BAD_REQUEST: return "Bad Request";
        case UNAUTHORIZED: return "Unauthorized";
        case FORBIDDEN: return "Forbidden";
        case NOT_FOUND: return "Not Found";
        case METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case CONFLICT: return "Conflict";
        case LENGTH_REQUIRED: return "Length Required";
        case PAYLOAD_TOO_LARGE: return "Payload Too Large";
        case UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
        
        // 5xx Server Error
        case INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case NOT_IMPLEMENTED: return "Not Implemented";
        case BAD_GATEWAY: return "Bad Gateway";
        
        default: return "Unknown Status";
        // clang-format on
    }
}

} // namespace http
