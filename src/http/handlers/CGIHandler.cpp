#include "config/LocationBlock.hpp"
#include "core/Server.hpp"
#include "http/Handler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace http {

static std::vector<std::string> buildEnvp(Request const &req);
static std::vector<std::string> buildArgv(Request const &req);
static int execute(std::vector<std::string> const &argv, std::vector<std::string> const &envp);
static void setupPipes(Request const &req, int pipe_fd[2]);

void CGIHandler::handle(Request const &req, Response &res) const {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    std::vector<std::string> const &argv = buildArgv(req);
    std::vector<std::string> const &envp = buildEnvp(req);
    int pipe_fd[2];
    if (pipe(pipe_fd)) {
        LOG_ERROR("CGIHandler::handle::pipe: " << strerror(errno));
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    int pid = fork();
    if (pid == -1) {
        LOG_ERROR("CGIHandler::handle::fork: " << strerror(errno));
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    if (pid == 0) {
        setupPipes(req, pipe_fd);
        execute(argv, envp);
        exit(127);
    } else {
        close(pipe_fd[1]);
        res.setBodyFromCgi(pipe_fd[0]);
    }
};

static void setupPipes(Request const &req, int pipe_fd[2]) {
    close(pipe_fd[0]);
    dup2(pipe_fd[1], STDOUT_FILENO);
    dup2(pipe_fd[1], STDERR_FILENO);
    close(pipe_fd[1]);
    if (req.body() >= 0) {
        dup2(req.body(), STDIN_FILENO);
        close(req.body());
    } else {
        dup2(core::Server::getNullFd(), STDIN_FILENO);
        close(core::Server::getNullFd());
    }
}

static std::vector<char *> toCStringVector(std::vector<std::string> const &v) {
    std::vector<char *> res;
    res.reserve(v.size() + 1);
    for (size_t i = 0; i < v.size(); i++) {
        res.push_back(const_cast<char *>(v[i].c_str()));
    }
    res.push_back(NULL);
    return res;
}

static int execute(std::vector<std::string> const &argv, std::vector<std::string> const &envp) {
    std::vector<char *> const &argv_ptrs = toCStringVector(argv);
    std::vector<char *> const &envp_ptrs = toCStringVector(envp);
    return execve(argv_ptrs[0], &argv_ptrs[0], &envp_ptrs[0]);
}

static std::vector<std::string> buildArgv(Request const &req) {
    std::vector<std::string> argv;
    argv.reserve(3);
    std::string script_path = req.resolvePath();
    if (req.location()->has("cgi_pass")) {
        std::vector<std::string> const &cgi_pass = req.location()->get("cgi_pass", req);
        if (!cgi_pass.empty()) {
            argv.push_back(cgi_pass[0]);
            argv.push_back(script_path);
        } else
            argv.push_back(script_path);
    } else
        argv.push_back(script_path);
    return argv;
}

std::string formatHeaderName(const std::string &name) {
    std::string cgi_name = "HTTP_";
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (c == '-') {
            cgi_name += '_';
        } else {
            cgi_name += std::toupper(c);
        }
    }
    return cgi_name;
}

static std::vector<std::string> buildEnvp(Request const &req) {
    std::vector<std::string> envp;

    envp.push_back("REQUEST_METHOD=" + std::string(RequestStartLine::methodToString(req.method())));
    envp.push_back("SERVER_PROTOCOL=" + req.version());
    envp.push_back("SCRIPT_NAME=" + req.path());
    envp.push_back("QUERY_STRING=" + req.queryString());

    if (req.headers().has("Content-Length")) {
        envp.push_back("CONTENT_LENGTH=" + req.headers().get("Content-Length"));
    }
    if (req.headers().has("Content-Type")) {
        envp.push_back("CONTENT_TYPE=" + req.headers().get("Content-Type"));
    }

    for (Headers::HeaderMap::const_iterator it = req.headers().begin(); it != req.headers().end();
         ++it) {
        std::string name = it->first;
        if (name != "Content-Length" && name != "Content-Type") {
            envp.push_back(formatHeaderName(name) + "=" + it->second);
        }
    }

    envp.push_back("SERVER_SOFTWARE=Webserv/1.0");
    // envp.push_back("SERVER_PORT=" + intToString(req.port()));
    envp.push_back("GATEWAY_INTERFACE=CGI/1.1");
    return envp;
}

} // namespace http
