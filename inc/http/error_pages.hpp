#pragma once

#include <string>
#include <sstream>
#include "HttpStatus.hpp"
#include "Response.hpp"
#include "ResponseContent.hpp"

namespace http {

namespace error_pages {

Response notFound(const std::string &httpVersion, const HeaderMap &headers);
Response forbidden(const std::string &httpVersion, const HeaderMap &headers);
Response internalServerError(const std::string &httpVersion, const HeaderMap &headers);

} // namespace error_pages

} // namespace http