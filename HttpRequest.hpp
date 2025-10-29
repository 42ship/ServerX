#pragma once

#include "Headers.hpp"
#include "RequestLine.hpp"

class Request {
public:
    RequestLine requestLine;
    http::Headers headers;
    void *body; // In future have body class
};
