#pragma once

#include "HttpResponse.hpp"
#include <string>

namespace http {

/**
 * @class ResponseBuilder
 * @brief Serializes an HttpResponse object into a raw HTTP response string.
 */
class ResponseBuilder {
public:
    static std::string build(const HttpResponse &response);
};

} // namespace http
