#pragma once

#include "ValidationResult.hpp"
#include <string>

namespace config {
class Block;
}

namespace http {
class HttpRequest; // forward decl
class MimeTypes;   // forward decl
} // namespace http

namespace utils {

std::string extractHeaderParam(const std::string &str, const std::string &toFind);

ValidationResult parseFilename(http::HttpRequest const &req, http::MimeTypes const &mime);

ValidationResult validateUploadPath(const std::string &path);

ValidationResult checkUploadLimit(const std::string &contentLength, config::Block const &s);

ValidationResult checkContentLength(std::string contentLen);

} // namespace utils
