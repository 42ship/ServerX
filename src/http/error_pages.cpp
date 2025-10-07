#include "http/error_pages.hpp"
#include <map>
#include <sstream>
#include <string>
#include <utility>

using namespace std;

namespace http {

namespace error_pages {

namespace {

struct Key {
    Status code;
    std::string message;
    bool operator<(Key const &o) const {
        return (code < o.code) || (code == o.code && message < o.message);
    }
};

typedef map<Status, vector<char> > ErrorPageCache;
typedef map<Key, vector<char> > JsonErrorPageCache;

static std::vector<char> createErrorBody(Status code, char const *message) {
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
    string const &s = body.str();
    return vector<char>(s.begin(), s.end());
}

static vector<char> const &getCacherErrorBody(Status code, char const *message) {
    static ErrorPageCache cache;

    ErrorPageCache::const_iterator it = cache.find(code);
    if (it != cache.end()) {
        return it->second;
    }
    cache.insert(make_pair(code, createErrorBody(code, message)));
    return cache[code];
}

static std::vector<char> createJsonErrorBody(Status code, char const *message) {
    std::ostringstream body;
    body << "{ \"" << code << "\": \"" << message << "\" }";
    string const &s = body.str();
    return vector<char>(s.begin(), s.end());
}

// typedef std::map<Key, std::vector<char> > JsonErrorPageCache;

static const std::vector<char> &getCacherJsonErrorBody(Status code, const char *message) {
    static JsonErrorPageCache cache;
    const char *m = message ? message : "";
    Key k;
    k.code = code;
    k.message = m;

    JsonErrorPageCache::const_iterator it = cache.find(k);
    if (it != cache.end())
        return it->second;

    return cache.insert(std::make_pair(k, createJsonErrorBody(code, m))).first->second;
}

} // namespace

HttpResponse generateErrorResponse(Status code, const std::string &httpVersion) {
    HttpResponse res(code, httpVersion);

    std::vector<char> const &body = getCacherErrorBody(code, res.generateResponsePhrase());
    res.setBodyInMemory(body, "text/html");
    return res;
}

HttpResponse generateJsonErrorResponse(Status code, const std::string &httpVersion,
                                       std::string message) {
    HttpResponse res(code, httpVersion, JSON);

    // Determine which message to use for the cached JSON body
    const std::string &msg = message.empty() ? std::string(res.generateResponsePhrase()) : message;

    // Get cached JSON body for this code-message combination
    const std::vector<char> &body = getCacherJsonErrorBody(code, msg.c_str());
    res.setBodyInMemory(body, "application/json");

    return res;
}

} // namespace error_pages

} // namespace http
