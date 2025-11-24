#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "http/Handler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
// Helper function to validate header name for CGI (RFC 7230)
bool isValidCgiHeaderName(std::string const &headerName) {
    if (headerName.empty()) {
        return false;
    }
    for (size_t i = 0; i < headerName.length(); ++i) {
        char c = headerName[i];
        // Valid header name characters: alphanumeric, hyphen, underscore, dot
        // RFC 7230: field-name = token
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') ||
              c == '-' || c == '_' || c == '.')) {
            return false;
        }
    }
    return true;
}

// Helper function to build CGI environment variables
std::vector<std::string> buildCgiEnvironment(http::Request const &req, int port) {
    std::vector<std::string> env;

    // CGI/1.1 Meta-Variables (RFC 3875)
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=ServerX/1.0");
    env.push_back("SERVER_PROTOCOL=" + req.version());

    // Request Meta-Variables
    env.push_back("REQUEST_METHOD=" + std::string(http::RequestStartLine::methodToString(req.method())));
    env.push_back("QUERY_STRING=" + req.queryString());
    env.push_back("SCRIPT_NAME=" + req.path());

    // PATH_INFO and PATH_TRANSLATED
    // For now, set to empty as most simple CGI scripts don't use extra path info
    // In a complete implementation, this would extract any path after the script filename
    env.push_back("PATH_INFO=");
    env.push_back("PATH_TRANSLATED=");

    // Server Meta-Variables
    std::ostringstream portStr;
    portStr << port;
    env.push_back("SERVER_PORT=" + portStr.str());

    if (req.server() && req.server()->has("server_name")) {
        std::vector<std::string> serverNames = req.server()->get("server_name", req);
        if (!serverNames.empty()) {
            env.push_back("SERVER_NAME=" + serverNames[0]);
        }
    }

    // Remote Address (RFC 3875 section 4.1.8)
    if (!req.remoteAddr().empty()) {
        env.push_back("REMOTE_ADDR=" + req.remoteAddr());
    } else {
        // Fallback for safety (should never happen)
        env.push_back("REMOTE_ADDR=127.0.0.1");
    }

    // Content Meta-Variables
    std::string contentType = req.headers().get("Content-Type");
    if (!contentType.empty()) {
        env.push_back("CONTENT_TYPE=" + contentType);
    }

    std::string contentLength = req.headers().get("Content-Length");
    if (!contentLength.empty()) {
        env.push_back("CONTENT_LENGTH=" + contentLength);
    }

    // HTTP_* Variables from request headers
    http::Headers::HeaderMap const &allHeaders = req.headers().getAll();
    for (http::Headers::HeaderMap::const_iterator it = allHeaders.begin();
         it != allHeaders.end(); ++it) {
        std::string headerName = it->first;

        // Validate header name (skip invalid headers)
        if (!isValidCgiHeaderName(headerName)) {
            continue;
        }

        // Skip headers that are already handled as special CGI variables
        if (headerName == "content-type" || headerName == "content-length") {
            continue;
        }

        // Convert header name to CGI format: uppercase and replace '-' with '_'
        std::string cgiName = "HTTP_";
        for (size_t i = 0; i < headerName.length(); ++i) {
            char c = headerName[i];
            if (c == '-') {
                cgiName += '_';
            } else if (c >= 'a' && c <= 'z') {
                cgiName += (c - 'a' + 'A');  // Convert to uppercase
            } else {
                cgiName += c;  // Keep uppercase, digits, underscore, dot as-is
            }
        }

        env.push_back(cgiName + "=" + it->second);
    }

    return env;
}

// Helper function to convert vector to char** for execve
char**  vectorToCharArray(std::vector<std::string> const &vec) {
    char**  arr = new char*[vec.size() + 1];
    for (size_t i = 0; i < vec.size(); ++i) {
        arr[i] = const_cast<char*>(vec[i].c_str());
    }
    arr[vec.size()] = NULL;
    return (arr);
}

}

namespace http {


void CGIHandler::handle(Request const &req, Response &res) const {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    std::string interpreterPath = req.location()->get("cgi_pass", req)[0];
    std::string scriptPath = req.resolvePath();
    int pipeFd[2];
    if (pipe(pipeFd)) {
        LOG_ERROR("CGIHandler::handle::pipe: " << strerror(errno));
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    int pid = fork();
    if (pid == -1) {
        LOG_ERROR("CGIHandler::handle::fork: " << strerror(errno));
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    if (pid == 0) {
        close(pipeFd[0]);
        dup2(pipeFd[1], STDOUT_FILENO);
        close(pipeFd[1]);
        char *argv[] = {const_cast<char *>(interpreterPath.c_str()), // argv[0] = /usr/bin/python3
                        const_cast<char *>(scriptPath.c_str()), // argv[1] = /var/www/script.py
                        NULL};

        // Build CGI environment
        int port = req.server() ? req.server()->port() : 0;
        std::vector<std::string> env = buildCgiEnvironment(req, port);
        char**  envp = vectorToCharArray(env);
        execve(interpreterPath.c_str(), argv, envp);
        LOG_ERROR("EXECVE");
        exit(127);
    }
    // Parent process (pid > 0)
    if (pid > 0) {
        close(pipeFd[1]);
        res.setBodyFromCgi(pipeFd[0]);
    }
}

} // namespace http
