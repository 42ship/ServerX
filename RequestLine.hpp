#pragma once

#include <string>

class RequestLine {
public:
    enum Method { GET, POST, PUT, DELETE, UNKNOWN };

    Method method;
    std::string uri;
    std::string path;
    std::string version;

    RequestLine static parse(std::string const &line);
};
