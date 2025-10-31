#include "http/HttpResponse.hpp"
#include "common/string.hpp"
#include <string>

namespace http {

HttpResponse::HttpResponse() : body(NULL) {}
HttpResponse::~HttpResponse() { delete body; }

void HttpResponse::setNoBody() {
    delete body;
    body = new NoBody;
    headers.erase("Content-Length");
    headers.erase("Content-Type");
}

void HttpResponse::setBodyInMemory(std::string const &data, std::string const &contentType) {
    delete body;
    body = new BodyInMemory(data);
    headers.add("Content-Length", utils::toString(body->size()));
    headers.add("Content-Type", contentType);
}

void HttpResponse::setBodyFromFile(std::string const &fpath, std::string const &contentType) {
    delete body;
    body = new FileBody(fpath);
    headers.add("Content-Length", utils::toString(body->size()));
    headers.add("Content-Type", contentType);
}

void HttpResponse::setBodyFromCgi(int pipeFd, std::string const &firstChunk) {
    delete body;
    body = new BodyFromCgi;
    (void)pipeFd;
    (void)firstChunk;
}

#if 0
namespace {

inline const char *getResponsePhrase(Status status) {
    switch (status) {
    case OK:
        return "OK";
    case CREATED:
        return "Created";
    case ACCEPTED:
        return "Accepted";
    case NO_CONTENT:
        return "No Content";
    case BAD_REQUEST:
        return "Bad Request";
    case UNAUTHORIZED:
        return "Unauthorized";
    case FORBIDDEN:
        return "Forbidden";
    case NOT_FOUND:
        return "Not Found";
    case METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

inline const char *getJsonResponsePhrase(Status status) {
    switch (status) {
    case OK:
        return "Upload successful";
    case CREATED:
        return "File uploaded successfully";
    case NO_CONTENT:
        return "\0";
    case BAD_REQUEST:
        return "Invalid request format";
    case UNAUTHORIZED:
        return "Authentication required";
    case FORBIDDEN:
        return "Permission denied";
    case NOT_FOUND:
        return "Not Found";
    case LENGTH_REQUIRED:
        return "Length Required";
    case PAYLOAD_TOO_LARGE:
        return "File size exceeds the allowed limit";
    case UNSUPPORTED_MEDIA_TYPE:
        return "Unsupported file type";
    case INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

} // namespace

void HttpResponse::setStatus(Status s) { statusCode_ = s; }

Status HttpResponse::getStatus() const { return statusCode_; }

std::string const &HttpResponse::getVersion() const { return httpVersion_; }

BodySourceType HttpResponse::getBodyType() const { return bodyType_; }

void HttpResponse::cleanupBody() {
    if (bodyType_ == BODY_IN_MEMORY && inMemoryBody.data != NULL) {
        delete inMemoryBody.data;
        inMemoryBody.data = NULL;
    }
}

std::string HttpResponse::buildHeaders() const {
    std::ostringstream oss;

    oss << httpVersion_ << " " << statusCode_ << " " << generateResponsePhrase() << "\r\n";
    for (HeaderMap::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n";
    return oss.str();
}

MessageType HttpResponse::getMessageType() const { return messageType_; }

char const *HttpResponse::generateResponsePhrase() const {
    if (messageType_ == JSON) {
        return http::getJsonResponsePhrase(statusCode_);
    }
    return http::getResponsePhrase(statusCode_);
}

std::ostream &operator<<(std::ostream &o, HttpResponse const &r) {
    o << "version(" << r.getVersion() << "); status(" << r.getStatus() << "); phrase("
      << r.generateResponsePhrase() << ")";
    return o;
}
#endif

} // namespace http
