#pragma once

#include <string>

namespace http {

class RequestLine {
public:
    enum Method { GET, POST, PUT, DELETE, UNKNOWN };

    Method method;
    std::string uri;
    std::string path;
    std::string version;

    static RequestLine parse(std::string const &line);
    static Method matchHttpMethod(std::string const &s);
    static char const *methodToString(Method m);
};

} // namespace http
