#include "http/Handler.hpp"

#include "common/filesystem.hpp"
#include "config/ServerBlock.hpp"
#include "config/arguments/Integer.hpp"

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

DefaultErrorHandler::DefaultErrorHandler(MimeTypes const &mime) : mimeTypes_(mime) {};

void DefaultErrorHandler::serveErrorFile(Response &response, const std::string &root, std::string fpath) const {
    if (fpath.empty()) {
        populateResponse(response);
        return;
    }

    if (fpath[0] == '.') {
        fpath = fpath.substr(1);
    } else if (fpath[0] == '/' || root.empty()) {
        response.setBodyFromFile(fpath, mimeTypes_.getMimeType(utils::getFileExtension(fpath)));
        return;
    }

    // prepend root if relative
    fpath = root + (root[root.size() - 1] == '/' ? "" : "/") + fpath;
    response.setBodyFromFile(fpath, mimeTypes_.getMimeType(utils::getFileExtension(fpath)));
}

void DefaultErrorHandler::handle(Request const &request, Response &response) const {
    CHECK_FOR_SERVER_AND_LOCATION(request, response);
    config::Integer status(response.status());
    std::string root = request.server()->root();
    std::string fpath;

    // try location-level error_page
    if (request.location()->has(status.getRawValue())) {
        fpath = request.location()->get(status.getRawValue())[0]->getRawValue();
        serveErrorFile(response, root, fpath);
        return;
    }

    // try server-level error_page
    if (request.server()->has(status.getRawValue())) {
        fpath = request.server()->get(status.getRawValue())[0]->getRawValue();
        serveErrorFile(response, root, fpath);
        return;
    }

    // fallback to default response
    populateResponse(response);
}


} // namespace http
