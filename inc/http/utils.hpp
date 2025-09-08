#pragma once

#include <ostream>

namespace utils {

enum HttpMethod { GET, POST, PUT, DELETE, UNKNOWN };

std::ostream &operator<<(std::ostream &o, HttpMethod);

HttpMethod matchHttpMethod(std::string const &);

std::string getFileExtension(std::string const &fpath);

} // namespace utils
