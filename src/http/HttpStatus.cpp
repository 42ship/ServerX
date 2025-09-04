#include "http/HttpStatus.hpp"

http::StatusCode::StatusCode() : code_(http::OK) {
}

http::StatusCode::StatusCode(Status code) : code_(code) {
}

http::StatusCode::StatusCode(const StatusCode &other) : code_(other.code_) {
}

http::StatusCode &http::StatusCode::operator=(const StatusCode &other) {
    if (this != &other) {
        code_ = other.code_;
    }
    return *this;
}

http::Status http::StatusCode::getCode() const {
    return code_;
}

const char *http::StatusCode::getMessage() const {
    return statusMessage(code_);
}

http::StatusCode::~StatusCode() {
}
