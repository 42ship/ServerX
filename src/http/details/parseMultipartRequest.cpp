#include "http/Handler.hpp"

#include <iostream>
#include <limits>
#include <sstream>
#include <ostream>
#include <string>

#include "http/HttpRequest.hpp"
#include "common/string.hpp"
#include "utils/Logger.hpp"

namespace http {

namespace details {

bool parseBody(HttpRequest &r, std::istringstream &s, const std::string &marker /* "--"+boundary */) {
    std::streampos pos = s.tellg();
    if (pos == std::streampos(-1)) {
        return false;
    }
    const std::string raw = s.str();
    const size_t start = static_cast<size_t>(pos);

    size_t p = raw.find("\r\n" + marker, start);
    if (p == std::string::npos) {
        p = raw.find(marker, start);
    }
    size_t end = (p == std::string::npos) ? raw.size() : p;
    if (end < start) {
        return false;
    }
    r.body.assign(raw, start, end - start);

    s.clear();
    s.seekg(static_cast<std::streamoff>(end + marker.size()), std::ios_base::beg);
    return true;
}

HttpRequest parse(std::istringstream &s, const std::string &boundary) {
    HttpRequest res;
    std::string tmp;
    // remove boundary from begin
    getline(s, tmp);
    if (!parseHeaders(res.headers, s) || !parseBody(res, s, boundary)) {
        res.status = INTERNAL_SERVER_ERROR;
    }
    return res;
}

}

} // namespace http