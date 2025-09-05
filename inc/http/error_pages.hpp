#pragma once

#include <string>
#include "HttpStatus.hpp"
#include "HttpResponse.hpp"

namespace http {

namespace error_pages {

HttpResponse generateErrorResponse(Status code, std::string const &httpVersion);

} // namespace error_pages

} // namespace http
