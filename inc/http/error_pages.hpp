#pragma once

#include "HttpResponse.hpp"
#include "HttpStatus.hpp"
#include <string>

namespace http {

namespace error_pages {

HttpResponse generateErrorResponse(Status code, std::string const &httpVersion);
HttpResponse generateJsonErrorResponse(Status code, std::string const &httpVersion,
                                       std::string message = "");
} // namespace error_pages

} // namespace http
