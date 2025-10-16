#include "http/HttpRequest.hpp"
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

using namespace std;

namespace http {
namespace details {

bool parseStartLine(HttpRequest &r, istringstream &s) {
    std::string method;
    if (!(s >> method >> r.uri >> r.version))
        return false;
    r.method = HttpRequest::matchHttpMethod(method);
    // Consume and discard the rest of the current line (e.g., the trailing CRLF).
    s.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return 1;
}

bool parseBody(HttpRequest &r, istringstream &s) {
    std::streampos pos = s.tellg();
    if (pos == -1) {
        return false;
    }

    size_t start = static_cast<size_t>(pos);
    size_t end = r.headers.getConentLength();
    r.body = s.str().substr(start, end);
    return true;
}

} // namespace details
} // namespace http
