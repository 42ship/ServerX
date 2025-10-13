#pragma once

#include "http/HttpStatus.hpp"
#include <map>
#include <ostream>
#include <string>

namespace http {

enum Method { GET, POST, PUT, DELETE, UNKNOWN };

/**
 * @class HttpRequest
 * @brief A data container for a parsed HTTP request.
 *
 * This object holds all the structured information from a client's raw
 * HTTP request message.
 */
class HttpRequest {
public:
    HttpRequest();
    HttpRequest& operator=(HttpRequest const &req);
    typedef std::map<std::string, std::string> HeaderMap;

    http::Status status;
    Method method;
    std::string uri;
    std::string path;
    std::string version;
    HeaderMap headers;
    std::string body;

    static HttpRequest parse(std::string const &);
    std::string getHeader(const std::string &key) const;

    static Method matchHttpMethod(std::string const &s);
    static char const *methodToString(Method);
};

std::ostream &operator<<(std::ostream &o, HttpRequest const &r);
std::ostream &operator<<(std::ostream &o, HttpRequest::HeaderMap const &r);
std::ostream &operator<<(std::ostream &o, Method);

namespace details {

bool parseStartLine(HttpRequest &r, std::istringstream &s);
bool parseHeaderLine(std::string const &line, std::pair<std::string, std::string> &p);
bool parseHeaders(HttpRequest::HeaderMap &m, std::istringstream &s);
bool parseBody(HttpRequest &r, std::istringstream &s);
std::string extractPathFUri(std::string const &uri);

} // namespace details

} // namespace http
