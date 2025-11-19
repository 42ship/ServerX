#include "http/Headers.hpp"
#include "common/string.hpp"
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

size_t Headers::getContentLength() const {
    static const std::string key = normalizeKey("content-length");
    HeaderMap::const_iterator it = map_.find(key);
    if (it == map_.end())
        return 0;
    return std::strtol(it->second.c_str(), NULL, 10);
}

bool Headers::parse(std::istringstream &s, Headers &res) {
    std::string line;

    while (getline(s, line)) {
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.resize(line.length() - 1);
        }
        if (line.empty()) {
            return true;
        }
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            return false;
        }
        std::string key = utils::trim(line.substr(0, colon_pos));
        if (key.empty())
            return false;
        std::string value = utils::trim(line.substr(colon_pos + 1));
        res.add(key, value);
    }
    return true;
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

bool Headers::parse(std::string &buffer, Headers &headers) {
    std::istringstream s(buffer);
    return parse(s, headers);
}

Headers Headers::parse(std::string &buffer) {
    std::istringstream s(buffer);
    return parse(s);
}

bool Headers::isContentChunked() const {
    std::string value;
    // TODO: add parsing to check if chunked exists there
    if (!get("Transfer-Encoding", value))
        return false;
    return true;
}

Headers &Headers::clear() {
    map_.clear();
    return *this;
}

Headers &Headers::erase(std::string const &key) {
    map_.erase(normalizeKey(key));
    return *this;
}

std::string Headers::toString() const {
    std::ostringstream oss;

    for (HeaderMap::const_iterator it = map_.begin(); it != map_.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    return oss.str();
}

bool Headers::has(std::string const &key) const { return map_.count(normalizeKey(key)); }

Headers::HeaderMap::const_iterator Headers::begin() const { return map_.begin(); }
Headers::HeaderMap::const_iterator Headers::end() const { return map_.end(); }

} // namespace http
