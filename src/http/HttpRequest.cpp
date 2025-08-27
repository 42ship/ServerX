#include "http/HttpRequest.hpp"
#include "http/utils.hpp"
#include <sstream>
#include <string>
#include <ostream>
#include <iostream>
#include <limits>

using namespace std;

namespace http {

using namespace utils;
using namespace http::detail;

HttpRequest::HttpRequest() : status(200) {
}

HttpRequest HttpRequest::parse(string const &buffer) {
    HttpRequest res;
    istringstream s(buffer);

    if (!parseStartLine(res, s) || !parseHeaders(res.headers, s) || !parseBody(res, s)) {
        res.status = 400;
    }
    return res;
}

std::ostream &operator<<(std::ostream &o, HttpRequest const &r) {
    o << "Method: (" << r.method << ")\nUri: (" << r.uri << ")\nVersion: (" << r.version << ")";
    return o;
}

ostream &operator<<(ostream &o, HttpRequest::HeaderMap const &r) {
    for (HttpRequest::HeaderMap::const_iterator it = r.begin(); it != r.end(); ++it) {
        o << it->first << ": " << it->second << "\n";
    }
    return o;
}

namespace detail {

bool parseStartLine(HttpRequest &r, istringstream &s) {
    std::string method;
    if (!(s >> method >> r.uri >> r.version))
        return false;
    r.method = matchHttpMethod(method);
    // Consume and discard the rest of the current line (e.g., the trailing CRLF).
    s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return 1;
}

bool parseHeaderLine(string line, pair<string, string> &p) {
    size_t column_pos = line.find(':');
    if (column_pos == string::npos)
        return false;
    size_t key_start = line.find_first_not_of(" \t\r\n");
    size_t key_end = line.find_last_not_of(" \t\r\n");
    if (key_start == string::npos || key_start > key_end)
        return false;
    size_t value_start = line.find_first_not_of(" \t\r\n", column_pos + 1);
    if (value_start == string::npos || value_start > key_end)
        return false;
    p.first = line.substr(key_start, column_pos - key_start);
    p.second = line.substr(value_start, key_end - value_start + 1);
    return true;
}

bool parseHeaders(HttpRequest::HeaderMap &m, istringstream &s) {
    string line;
    pair<string, string> p;

    while (getline(s, line)) {
        if (line.size() == 1 && line[0] == '\r')
            break;
        if (!parseHeaderLine(line, p))
            return false;
        m.insert(p);
    }
    return 1;
}

bool parseBody(HttpRequest &r, istringstream &s) {
    (void)r;
    (void)s;
    return 1;
}

} // namespace detail

} // namespace http
