#include "http/utils.hpp"

namespace utils {

HttpMethod matchHttpMethod(std::string const &s) {
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

std::ostream &operator<<(std::ostream &o, HttpMethod m) {
    switch (m) {
    case GET:
        o << "GET";
        break;
    case POST:
        o << "POST";
        break;
    case PUT:
        o << "PUT";
        break;
    case DELETE:
        o << "DELETE";
        break;
    case UNKNOWN:
        o << "UNKNOWN";
        break;
    }
    return o;
}

} // namespace utils
