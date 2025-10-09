#pragma once

#include <string>
#include "HttpStatus.hpp"
#include "HttpResponse.hpp"

namespace http {

namespace error_pages {

/**
 * @brief Generates a standard HTML error response for a given HTTP status code.
 *
 * This function constructs an HTTP response containing a basic HTML error page
 * with the appropriate status line and reason phrase.  
 * It is typically used for plain-text or HTML-based responses (e.g. 404, 500).
 *
 * @param code         The HTTP status code (e.g. NOT_FOUND, INTERNAL_SERVER_ERROR).
 * @param httpVersion  The HTTP version string (e.g. "HTTP/1.1").
 * @return             An HttpResponse object representing the error response.
 */
HttpResponse generateErrorResponse(Status code, std::string const &httpVersion);

/**
 * @brief Generates a JSON error response with optional custom message.
 *
 * Uses a cached JSON body for the given status code and message.
 * If no custom message is provided, the standard HTTP reason phrase
 * (e.g. "Not Found") will be used instead.
 *
 * @param code         The HTTP status code (e.g. BAD_REQUEST, NOT_FOUND).
 * @param httpVersion  The HTTP version string (e.g. "HTTP/1.1").
 * @param message      Optional custom message providing more error details.
 * @return             An HttpResponse object containing a JSON error body.
 */
HttpResponse generateJsonErrorResponse(Status code, std::string const &httpVersion, const std::string &message = "");

} // namespace error_pages

} // namespace http
