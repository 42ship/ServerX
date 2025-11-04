#include "http/Handler.hpp"
#include <iostream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>

namespace http {

typedef std::map<HttpStatus, std::string> ErrorPageCache;

static std::string createErrorBody(HttpStatus code, char const *message) {
    std::ostringstream body;
    body << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\" />\n"
         << "    <title>" << message << "</title>\n"
         << "    <style>\n"
         << "        body {\n"
         << "            font-family: system-ui, -apple-system, sans-serif;\n"
         << "            background-color: var(--bg);\n"
         << "            color: var(--text);\n"
         << "            display: flex;\n"
         << "            align-items: center;\n"
         << "            justify-content: center;\n"
         << "            min-height: 100vh;\n"
         << "            margin: 0;\n"
         << "            text-align: center;\n"
         << "            transition: background 0.3s ease, color 0.3s ease;\n"
         << "        }\n"
         << "        h1 {\n"
         << "            font-size: 4rem;\n"
         << "            margin: 0;\n"
         << "            color: var(--primary);\n"
         << "        }\n"
         << "        p {\n"
         << "            font-size: 1.2rem;\n"
         << "            margin: 1rem 0;\n"
         << "            color: var(--muted);\n"
         << "        }\n"
         << "        .error-container {\n"
         << "            max-width: 500px;\n"
         << "            padding: 2rem;\n"
         << "            background: transparent;\n"
         << "            border-radius: 12px;\n"
         << "        }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <div class=\"error-container\">\n"
         << "    <h1>" << code << "</h1>\n"
         << "    <p>" << message << "</p>\n"
         << "  </div>\n"
         << "</body>\n";
    return body.str();
}

static std::string const &getCacherErrorBody(HttpStatus code, char const *message) {
    static ErrorPageCache cache;

    ErrorPageCache::const_iterator it = cache.find(code);
    if (it != cache.end()) {
        return it->second;
    }
    cache.insert(make_pair(code, createErrorBody(code, message)));
    return cache[code];
}

void DefaultErrorHandler::populateResponse(Response &response) {
    std::string const &body =
        getCacherErrorBody(response.status(), response.reasonPhrase().c_str());
    response.setBodyInMemory(body, "text/html");
}

} // namespace http
