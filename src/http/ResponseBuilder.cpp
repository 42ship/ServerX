#include "http/ResponseBuilder.hpp"

#include <sstream>

namespace http {

const std::string ResponseBuilder::build(const Response &res) const {
    std::ostringstream out;
    out << res.getHttpVersion() << " " << res.getStatusCode().getCode() << " "
        << res.getStatusCode().getMessage() << "\r\n"
        << "Content-Type: " << res.getContent().getType() << "\r\n"
        << "Content-Length: " << res.getContent().getContentLength() << "\r\n";

    const HeaderMap &headers = res.getHeaders();
    for (HeaderMap::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        out << it->first << ": " << it->second << "\r\n";
    }

    out << "\r\n" << res.getContent().getBody();

    return out.str();
}

} // namespace http