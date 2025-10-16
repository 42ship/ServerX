#include "http/Body.hpp"

namespace http {

Body::Body() : type_(BODY_NONE) {}

void Body::setNoBody() { type_ = BODY_NONE; }

void Body::setBodyFromString(std::string &data) {
    inMemoryBody.data = &data;
    inMemoryBody.sent = 0;
    type_ = BODY_IN_MEMORY;
}

void Body::setBodyFromFile(int fd, size_t size) {
    fileBody.fd = fd;
    fileBody.totalSize = size;
    fileBody.sent = 0;
    type_ = BODY_FROM_FILE;
}

void Body::setBodyFromCgi(int pipe_fd) {
    cgiBody.pipe_fd = pipe_fd;
    type_ = BODY_FROM_CGI;
}

Body::BodyType Body::getBodyType() const { return type_; }

} // namespace http
