#include "http/HttpRequest.hpp"
#include <sstream>
#include <ostream>

using namespace std;

namespace http {

using namespace details;

HttpRequest::HttpRequest() : status(200) {
}

HttpRequest HttpRequest::parse(string const &buffer) {
    HttpRequest res;
    istringstream s(buffer);

    if (!parseStartLine(res, s) || !parseHeaders(res.headers, s) || !parseBody(res, s)) {
        res.status = 400;
    }
    res.path = extractPathFUri(res.uri);
    return res;
}

std::ostream &operator<<(std::ostream &o, HttpRequest const &r) {
    o << "▶️  " << r.method << " " << r.uri << " " << r.version << "\n";
    o << "\tStatus: " << r.status << "\n";
    o << "\tPath: " << r.path << "\n";
    return o;
}

ostream &operator<<(ostream &o, HttpRequest::HeaderMap const &r) {
    for (HttpRequest::HeaderMap::const_iterator it = r.begin(); it != r.end(); ++it) {
        o << it->first << ": " << it->second << "\n";
    }
    return o;
}

namespace details {

std::string extractPathFUri(std::string const &uri) {
    return uri.substr(0, uri.find_last_not_of("#?") + 1);
}

} // namespace details

} // namespace http
