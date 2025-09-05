#pragma once

#include "http/utils.hpp"
#include "http/HttpStatus.hpp"
#include <string>
#include <map>
#include <ostream>

namespace http {

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
    typedef std::map<std::string, std::string> HeaderMap;

    http::Status status;
    utils::HttpMethod method;
    std::string uri;
    std::string path;
    std::string version;
    HeaderMap headers;
    std::string body;

    static HttpRequest parse(std::string const &);
};

std::ostream &operator<<(std::ostream &o, HttpRequest const &r);
std::ostream &operator<<(std::ostream &o, HttpRequest::HeaderMap const &r);

namespace details {

bool parseStartLine(HttpRequest &r, std::istringstream &s);
bool parseHeaderLine(std::string line, std::pair<std::string, std::string> &p);
bool parseHeaders(HttpRequest::HeaderMap &m, std::istringstream &s);
bool parseBody(HttpRequest &r, std::istringstream &s);
std::string extractPathFUri(std::string const &uri);

} // namespace details

} // namespace http
