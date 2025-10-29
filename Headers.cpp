#include "Headers.hpp"
#include "string.hpp"
#include <algorithm>
#include <sstream>

namespace http {

Headers::Headers() {}

void Headers::normalizeKey(std::string &key) {
  std::transform(key.begin(), key.end(), key.begin(), ::tolower);
}

std::string Headers::normalizeKey(std::string const &key) {
  std::string res = key;
  normalizeKey(res);
  return res;
}

Headers &Headers::add(std::string const &key, std::string const &value) {
  if (key.empty())
    return *this;
  map_[normalizeKey(key)] = value;
  return *this;
}

bool Headers::get(std::string const &key, std::string &value) const {
  HeaderMap::const_iterator it = map_.find(normalizeKey(key));
  if (it == map_.end())
    return false;
  value = it->second;
  return true;
}

std::string Headers::get(std::string const &key) const {
  std::string res;
  get(key, res);
  return res;
}

size_t Headers::getConentLength() const {
  static const std::string key = normalizeKey("content-length");
  HeaderMap::const_iterator it = map_.find(key);
  if (it == map_.end())
    return 0;
  return std::strtol(it->second.c_str(), NULL, 10);
}

Headers Headers::parse(std::istringstream &s) {
  Headers res;
  std::string line;

  while (getline(s, line) && !line.empty() && line[0] != 'r') {
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
      continue;
    }
    std::string key = utils::trim(line.substr(0, colon_pos));
    if (key.empty())
      continue;
    std::string value = utils::trim(line.substr(colon_pos + 1));
    res.add(key, value);
  }
  return res;
}

Headers Headers::parse(std::string &buffer) {
  std::istringstream s(buffer);
  return parse(s);
}

} // namespace http
