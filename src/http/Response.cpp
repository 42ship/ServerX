#include "http/Response.hpp"
#include "common/string.hpp"
#include "http/ResponseBody.hpp"
#include <string>

namespace http {

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
    std::string startLine = startLine_.protocol + " " + utils::toString(startLine_.statusCode) +
                            " " + startLine_.reasonPhrase + "\r\n";
    buffer.insert(buffer.end(), startLine.begin(), startLine.end());

    std::string headerLines = headers_.toString();
    buffer.insert(buffer.end(), headerLines.begin(), headerLines.end());

    buffer.push_back('\r');
    buffer.push_back('\n');
}

Response &Response::status(HttpStatus statusCode) {
    startLine_.statusCode = statusCode;
    startLine_.reasonPhrase = getReasonPhrase(statusCode);
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
