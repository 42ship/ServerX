#include "http/HttpRequest.hpp"
#include "http/HttpStatus.hpp"
#include <ostream>
#include <sstream>

using namespace std;

namespace http {

using namespace details;

HttpRequest::HttpRequest() : status(OK) {}

HttpRequest::HttpRequest(HttpRequest const &req)
    : status(req.status),
      method(req.method),
      uri(req.uri),
      path(req.path),
      version(req.version),
      headers(req.headers),
      body(req.body) {}

HttpRequest &HttpRequest::operator=(HttpRequest const &req) {
    status = req.status;
    method = req.method;
    uri = req.uri;
    path = req.path;
    version = req.version;
    headers = req.headers;
    body = req.body;

    return *this;
}

bool parsePath(HttpRequest &res) {
    res.path = extractPathFUri(res.uri);
    return res.path.find("./") == std::string::npos;
}

HttpRequest HttpRequest::parse(string const &buffer) {
    HttpRequest res;
    istringstream s(buffer);

    if (!parseStartLine(res, s) || !parsePath(res) || !parseHeaders(res.headers, s) ||
        !parseBody(res, s)) {
        res.status = BAD_REQUEST;
    }
    return res;
}

std::string HttpRequest::getHeader(const std::string &key) const {
    std::map<std::string, std::string>::const_iterator it = headers.find(key);
    if (it != headers.end())
        return it->second;
    return std::string();
}

Method HttpRequest::matchHttpMethod(std::string const &s) {
    if (s == "GET")
        return GET;
    if (s == "POST")
        return POST;
    if (s == "PUT")
        return PUT;
    if (s == "DELETE")
        return DELETE;
    return UNKNOWN;
}

char const *HttpRequest::methodToString(Method m) {
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

std::ostream &operator<<(std::ostream &o, HttpRequest const &r) {
    o << "▶️  " << r.method << " " << r.uri << " " << r.version << "\n";
    o << "\tStatus: " << r.status << "\n";
    o << "\tPath: " << r.path << "\n";
    return o;
}

ostream &operator<<(ostream &o, HttpRequest::HeaderMap const &r) {
    for (HttpRequest::HeaderMap::const_iterator it = r.begin(); it != r.end(); ++it) {
        o << it->first << ": " << it->second << "\n";
    }
    return o;
}

std::ostream &operator<<(std::ostream &o, Method m) {
    o << HttpRequest::methodToString(m);
    return o;
}

namespace details {

std::string extractPathFUri(std::string const &uri) {
    return uri.substr(0, uri.find_first_of("#?"));
}

} // namespace details

} // namespace http
