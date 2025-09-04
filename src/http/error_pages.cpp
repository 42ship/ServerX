#include "http/error_pages.hpp"

namespace http {

namespace error_pages {

namespace {
Response errorPageHtml(const std::string &httpVersion, const HeaderMap &headers,
                       const StatusCode &code) {
    std::ostringstream body;
    body << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\" />\n"
         << "    <title>Error</title>\n"
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
         << "    <h1>" << code.getCode() << "</h1>\n"
         << "    <p>" << code.getMessage() << "</p>\n"
         << "  </div>\n"
         << "</body>\n";

    return Response(httpVersion, headers, code, ResponseContent(body.str(), "text/html"));
}
} // namespace
Response notFound(const std::string &httpVersion, const HeaderMap &headers) {
    return errorPageHtml(httpVersion, headers, StatusCode(NOT_FOUND));
}
Response forbidden(const std::string &httpVersion, const HeaderMap &headers) {

    return errorPageHtml(httpVersion, headers, StatusCode(FORBIDDEN));
}
Response internalServerError(const std::string &httpVersion, const HeaderMap &headers) {

    return errorPageHtml(httpVersion, headers, StatusCode(INTERNAL_SERVER_ERROR));
}
} // namespace error_pages
} // namespace http