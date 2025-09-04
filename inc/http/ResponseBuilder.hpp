#pragma once

#include <string>
#include "http/Response.hpp"

namespace http {

/**
 * @class ResponseBuilder
 * @brief Serializes an HttpResponse object into a raw HTTP response string.
 */
class ResponseBuilder {
public:
    /**
     * @brief Builds a complete HTTP response message as a string.
     *
     * Constructs the raw HTTP response string to be sent to the client.
     *
     * The response format:
     * - Status line (HTTP version, status code, reason phrase)
     * - Default headers (Content-Type, Content-Length)
     * - Additional headers from the HeaderMap
     * - Empty line ("\r\n") separating headers from the body
     * - Response body
     *
     * @param res The Response object containing status, headers, and body.
     * @return A complete HTTP response as a single std::string.
     */
    const std::string build(const Response &res) const;
};

} // namespace http
