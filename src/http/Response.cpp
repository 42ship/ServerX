#include "http/Response.hpp"

namespace http {

Response::Response() : httpVersion_("HTTP/1.1"), statusCode_(OK), content_() {
    headers_.empty();
    // buildResponseStream();
}

Response::Response(const std::string &httpVersion, const HeaderMap &headers,
                         const StatusCode &statusCode, const ResponseContent &content)
    : httpVersion_(httpVersion), headers_(headers), statusCode_(statusCode), content_(content) {
    // buildResponseStream();
}

Response::Response(const Response &other)
    : httpVersion_(other.httpVersion_), headers_(other.headers_), statusCode_(other.statusCode_),
      content_(other.content_) {
    // buildResponseStream();
}

Response &Response::operator=(const Response &other) {
    if (this != &other) {
        httpVersion_ = other.httpVersion_;
        headers_ = other.headers_;
        statusCode_ = other.statusCode_;
        content_ = other.content_;
        // buildResponseStream();
    }
    return *this;
}

const std::string &Response::getHttpVersion() const {
    return httpVersion_;
}

const HeaderMap &Response::getHeaders() const {
    return headers_;
}

const ResponseContent &Response::getContent() const {
    return content_;
}

const StatusCode &Response::getStatusCode() const {
    return statusCode_;
}

// std::string Response::toString() const {
//     return respStream_.str();
// }

Response::~Response() {
}

} // namespace