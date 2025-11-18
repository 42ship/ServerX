#include "http/Response.hpp"
#include "common/string.hpp"
#include "http/ResponseBody.hpp"
#include <string>

namespace http {

namespace {

inline const char *getReasonPhrase(HttpStatus status) {
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
    case PAYLOAD_TOO_LARGE:
        return "Payload too large";
    default:
        return "Unknown";
    }
}
} // namespace

ResponseStartLine::ResponseStartLine() : protocol("HTTP/1.1"), statusCode(OK), reasonPhrase("OK") {}

Response::Response() : body_(NULL) {}
Response::~Response() { delete body_; }

IResponseBody *Response::body() const { return body_; }
Response &Response::setNoBody() {
    delete body_;
    body_ = new NoBody;
    headers_.erase("Content-Length");
    headers_.erase("Content-Type");
    return *this;
}

Response &Response::setBodyInMemory(std::string const &data, std::string const &contentType) {
    delete body_;
    body_ = new BodyInMemory(data);
    headers_.add("Content-Length", utils::toString(body_->size()));
    headers_.add("Content-Type", contentType);
    return *this;
}

Response &Response::setBodyFromFile(std::string const &fpath, std::string const &contentType) {
    delete body_;
    body_ = new FileBody(fpath);
    headers_.add("Content-Length", utils::toString(body_->size()));
    headers_.add("Content-Type", contentType);
    return *this;
}

Response &Response::setBodyFromCgi(int pipeFd) {
    delete body_;
    body_ = new BodyFromCgi(pipeFd);
    return *this;
}

void Response::buildHeaders(std::vector<char> &buffer) const {
    std::string start_line = startLine_.protocol + " " + utils::toString(startLine_.statusCode) +
                             " " + startLine_.reasonPhrase + "\r\n";
    buffer.insert(buffer.end(), start_line.begin(), start_line.end());

    std::string header_lines = headers_.toString();
    buffer.insert(buffer.end(), header_lines.begin(), header_lines.end());

    buffer.push_back('\r');
    buffer.push_back('\n');
}

Response &Response::status(HttpStatus status) {
    startLine_.statusCode = status;
    startLine_.reasonPhrase = getReasonPhrase(status);
    return *this;
}

Response &Response::clear() {
    delete body_;
    body_ = NULL;
    status(OK);
    headers_.clear();
    return *this;
}
Headers &Response::headers() { return headers_; }
Headers const &Response::headers() const { return headers_; }
HttpStatus Response::status() const { return startLine_.statusCode; }
std::string const &Response::protocol() const { return startLine_.protocol; }
std::string const &Response::reasonPhrase() const { return startLine_.reasonPhrase; }

Response &Response::status(HttpStatus statusCode, std::string const &customError) {
    status(statusCode);
    customError_ = customError;
    return *this;
}

} // namespace http
