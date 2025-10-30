#include "http/RequestLine.hpp"
#include <limits>
#include <sstream>

namespace http {

RequestLine RequestLine::parse(std::string const &line) {
    RequestLine rl;
    std::string method;
    std::istringstream s(line);
    if (!(s >> method >> rl.uri >> rl.version))
        return rl;
    rl.method = matchHttpMethod(method);
    // Consume and discard the rest of the current line (e.g., the trailing CRLF).
    s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return rl;
}
RequestLine::Method RequestLine::matchHttpMethod(std::string const &s) {
    if (s == "GET")
        return GET;
    if (s == "POST")
        return POST;
    if (s == "PUT")
        return PUT;
    if (s == "DELETE")
        return DELETE;
    return RequestLine::UNKNOWN;
}

char const *RequestLine::methodToString(Method m) {
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

} // namespace http
