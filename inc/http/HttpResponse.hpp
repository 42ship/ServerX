#pragma once

#include <vector>
#include <map>
#include <string>

namespace http {

/**
 * @class HttpResponse
 * @brief A data container for an HTTP response.
 *
 * This object is populated by a handler and then serialized into a raw
 * HTTP response string by the ResponseBuilder.
 */
class HttpResponse {
public:
    typedef std::map<std::string, std::string> HeaderMap;

    std::string version;
    int status;
    std::string responsePhrase;
    HeaderMap headers;
    std::vector<char> body;
};

} // namespace http
