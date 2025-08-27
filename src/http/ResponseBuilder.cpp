#include "http/ResponseBuilder.hpp"
#include "http/HttpResponse.hpp"
#include <ostream>
#include <sstream>

using namespace std;

namespace http {

string ResponseBuilder::build(const HttpResponse &response) {
    ostringstream oss;

    oss << response.version << " " << response.status << " " << response.responsePhrase << "\r\n";

    HttpResponse::HeaderMap const &headers = response.headers;

    for (HttpResponse::HeaderMap::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }
    if (headers.find("Content-Length") == headers.end()) {
        oss << "Content-Length: " << response.body.size() << "\r\n";
    }

    oss << "\r\n";
    // TODO: Add body
    return oss.str();
}

} // namespace http
