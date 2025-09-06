#include "http/HttpStatus.hpp"
#include "http/HttpResponse.hpp"
#include "config/utils.hpp"
#include <string>
#include <sstream>

namespace http {

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

} // namespace

HttpResponse::HttpResponse() : statusCode_(INTERNAL_SERVER_ERROR), bodyType_(BODY_NONE) {
}

HttpResponse::HttpResponse(Status code, std::string const &httpVersion)
    : httpVersion_(httpVersion), statusCode_(code), bodyType_(BODY_NONE) {
}

HttpResponse::HttpResponse(HttpResponse const &rhs)
    : httpVersion_(rhs.httpVersion_),
      statusCode_(rhs.statusCode_),
      headers_(rhs.headers_),
      bodyType_(rhs.bodyType_) {
    if (bodyType_ == BODY_IN_MEMORY) {
        inMemoryBody = rhs.inMemoryBody;
        if (inMemoryBody.data != NULL)
            inMemoryBody.data = new std::vector<char>(*rhs.inMemoryBody.data);
    } else if (bodyType_ == BODY_FROM_FILE)
        fileBody = rhs.fileBody;
    else if (bodyType_ == BODY_FROM_CGI)
        cgiBody = rhs.cgiBody;
}

HttpResponse const &HttpResponse::operator=(HttpResponse const &rhs) {
    if (this == &rhs)
        return *this;
    cleanupBody();
    httpVersion_ = rhs.httpVersion_;
    statusCode_ = rhs.statusCode_;
    headers_ = rhs.headers_;
    bodyType_ = rhs.bodyType_;
    if (rhs.bodyType_ == BODY_IN_MEMORY) {
        inMemoryBody = rhs.inMemoryBody;
        if (inMemoryBody.data != NULL)
            inMemoryBody.data = new std::vector<char>(*rhs.inMemoryBody.data);
    } else if (rhs.bodyType_ == BODY_FROM_FILE)
        fileBody = rhs.fileBody;
    else if (rhs.bodyType_ == BODY_FROM_CGI)
        cgiBody = rhs.cgiBody;
    return *this;
}

HttpResponse::~HttpResponse() {
    cleanupBody();
}

void HttpResponse::setStatus(Status s) {
    statusCode_ = s;
}

void HttpResponse::setNoBody() {
    cleanupBody();
    bodyType_ = BODY_NONE;
    statusCode_ = NO_CONTENT;
    headers_.erase("Content-Length");
    headers_.erase("Content-Type");
}

void HttpResponse::setBodyInMemory(std::vector<char> const &body, std::string const &mimeType) {
    cleanupBody();
    bodyType_ = BODY_IN_MEMORY;
    inMemoryBody.data = new std::vector<char>(body);
    inMemoryBody.sent = 0;
    headers_["Content-Length"] = utils::toString(body.size());
    headers_["Content-Type"] = mimeType;
}

void HttpResponse::setBodyFromFile(int fd, size_t size) {
    cleanupBody();
    bodyType_ = BODY_FROM_FILE;
    fileBody.fd = fd;
    fileBody.sent = 0;
    fileBody.totalSize = size;
    headers_["Content-Length"] = utils::toString(size);
}

void HttpResponse::setBodyFromFile(int fd, size_t size, std::string const &mimeType) {
    setBodyFromFile(fd, size);
    headers_["Content-Type"] = mimeType;
}

void HttpResponse::setBodyFromCgi(int pipe_fd) {
    cleanupBody();
    bodyType_ = BODY_FROM_CGI;
    cgiBody.pipe_fd = pipe_fd;
}

Status HttpResponse::getStatus() const {
    return statusCode_;
}

char const *HttpResponse::getResponsePhrase() const {
    return http::getResponsePhrase(statusCode_);
}

HeaderMap &HttpResponse::getHeaders() {
    return headers_;
}

HeaderMap const &HttpResponse::getHeaders() const {
    return headers_;
}

std::string const &HttpResponse::getVersion() const {
    return httpVersion_;
}

BodySourceType HttpResponse::getBodyType() const {
    return bodyType_;
}

void HttpResponse::cleanupBody() {
    if (bodyType_ == BODY_IN_MEMORY && inMemoryBody.data != NULL) {
        delete inMemoryBody.data;
        inMemoryBody.data = NULL;
    }
}

std::string HttpResponse::buildHeaders() const {
    std::ostringstream oss;

    oss << httpVersion_ << " " << statusCode_ << " " << getResponsePhrase() << "\r\n";
    for (HeaderMap::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    oss << "\r\n";
    return oss.str();
}

std::ostream &operator<<(std::ostream &o, HttpResponse const &r) {
    o << "version(" << r.getVersion() << "); status(" << r.getStatus() << "); phrase("
      << r.getResponsePhrase() << ")";
    return o;
}

} // namespace http
