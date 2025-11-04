#include "http/Request.hpp"
#include <sstream>

namespace http {

RequestStartLine::RequestStartLine() : method(RequestStartLine::UNKNOWN), version("HTTP/1.1") {}

RequestStartLine RequestStartLine::parse(std::string const &line) {
    RequestStartLine rsl;
    std::istringstream iss(line);
    std::string methodStr;

    if (!(iss >> methodStr >> rsl.uri >> rsl.version)) {
        rsl.method = UNKNOWN;
        return rsl;
    }

    rsl.method = matchHttpMethod(methodStr);

    size_t queryPos = rsl.uri.find('?');

    if (queryPos == std::string::npos) {
        rsl.path = rsl.uri;
        rsl.queryString = "";
    } else {
        rsl.path = rsl.uri.substr(0, queryPos);
        rsl.queryString = rsl.uri.substr(queryPos + 1);
    }

    return rsl;
}

RequestStartLine::Method RequestStartLine::matchHttpMethod(std::string const &s) {
    if (s == "GET")
        return GET;
    if (s == "POST")
        return POST;
    if (s == "PUT")
        return PUT;
    if (s == "DELETE")
        return DELETE;
    return RequestStartLine::UNKNOWN;
}

char const *RequestStartLine::methodToString(Method m) {
    switch (m) {
    case GET:
        return "GET";
    case POST:
        return "POST";
    case PUT:
        return "PUT";
    case DELETE:
        return "DELETE";
    default:
        return "UNKNOWN";
    }
}

Request::Request() : body_(NULL), location_(NULL), server_(NULL) {}

bool Request::wantsJson() const { return headers_.get("Accept") == "application/json"; }

void Request::clear() {
    headers_.clear();
    requestLine_.method = RequestStartLine::UNKNOWN;
    requestLine_.uri.clear();
    requestLine_.path.clear();
    requestLine_.version.clear();
    location_ = NULL;
    server_ = NULL;
}

// clang-format off
config::LocationBlock const *Request::location() const { return location_; }
config::ServerBlock const *Request::server() const { return server_; }
std::string const &Request::uri() const { return requestLine_.uri; }
std::string const &Request::path() const { return requestLine_.path; }
std::string const &Request::queryString() const { return requestLine_.queryString; }
std::string const &Request::version() const { return requestLine_.version; }
RequestStartLine::Method Request::method() const { return requestLine_.method; }
Headers &Request::headers() { return headers_; }
Headers const &Request::headers() const { return headers_; }
Request &Request::location(config::LocationBlock const *location) {location_ = location;return *this;}
Request &Request::server(config::ServerBlock const *server) {server_ = server;return *this;}
Request &Request::method(RequestStartLine::Method method) {requestLine_.method = method;return *this;}
Request &Request::uri(std::string const &uri) {requestLine_.uri = uri;return *this;}
Request &Request::version(std::string const &version) {requestLine_.version= version;return *this;}
// clang-format on

} // namespace http
