#include "http/HttpStatus.hpp"
#include <cstdlib>

namespace http {

HttpStatus toHttpStatus(int code) {
    switch (code) {
        // clang-format off
        case 200: return OK;
        case 201: return CREATED;
        case 202: return ACCEPTED;
        case 204: return NO_CONTENT;
        case 400: return BAD_REQUEST;
        case 401: return UNAUTHORIZED;
        case 403: return FORBIDDEN;
        case 404: return NOT_FOUND;
        case 405: return METHOD_NOT_ALLOWED;
        case 409: return CONFLICT;
        case 411: return LENGTH_REQUIRED;
        case 413: return PAYLOAD_TOO_LARGE;
        case 415: return UNSUPPORTED_MEDIA_TYPE;
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
        case OK: return "OK";
        case CREATED: return "Created";
        case ACCEPTED: return "Accepted";
        case NO_CONTENT: return "No Content";
        case BAD_REQUEST: return "Bad Request";
        case UNAUTHORIZED: return "Unauthorized";
        case FORBIDDEN: return "Forbidden";
        case NOT_FOUND: return "Not Found";
        case METHOD_NOT_ALLOWED: return "Method Not Allowed";
        case CONFLICT: return "Conflict";
        case LENGTH_REQUIRED: return "Length Required";
        case PAYLOAD_TOO_LARGE: return "Payload Too Large";
        case UNSUPPORTED_MEDIA_TYPE: return "Unsupported Media Type";
        case INTERNAL_SERVER_ERROR: return "Internal Server Error";
        case NOT_IMPLEMENTED: return "Not Implemented";
        case BAD_GATEWAY: return "Bad Gateway";
        default: return "Unknown Status";
        // clang-format on
    }
}

} // namespace http
