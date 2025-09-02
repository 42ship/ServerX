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

    if (response.body.size() > 0)
        oss << "Content-Length: " << response.body.size() << "\r\n";
    for (HttpResponse::HeaderMap::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        oss << it->first << ": " << it->second << "\r\n";
    }

    oss << "\r\n";
    if (response.body.size() > 0)
        oss.write(&response.body[0], response.body.size());
    return oss.str();
}

} // namespace http
