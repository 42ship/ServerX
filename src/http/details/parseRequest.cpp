#include "http/HttpRequest.hpp"
#include <limits>
#include <sstream>
#include <string>
#include <iostream>

#include "utils/utils.hpp"

using namespace std;

namespace http {
namespace details {

bool parseStartLine(HttpRequest &r, istringstream &s) {
    std::string method;
    if (!(s >> method >> r.uri >> r.version))
        return false;
    r.method = utils::matchHttpMethod(method);
    // Consume and discard the rest of the current line (e.g., the trailing CRLF).
    s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return 1;
}

bool parseHeaderLine(string const &line, pair<string, string> &p) {
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
    std::streampos pos = s.tellg();
    if (pos == -1) {
        return false;
    }

    size_t start = static_cast<size_t>(pos);
    size_t end;
    std::string len = r.headers["Content-Length"];
    if (len == "") {
        return true;
    }
    else {
        end = utils::fromString<size_t>(len);
    }
    r.body = s.str().substr(start, end);
    return true;
}

} // namespace details
} // namespace http
