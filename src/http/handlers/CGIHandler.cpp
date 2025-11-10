#include "config/LocationBlock.hpp"
#include "http/Handler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

namespace {
// Helper function to build CGI environment variables
std::vector<std::string> buildCgiEnvironment(http::Request const &req, int port) {
    std::vector<std::string> env;
    (void)req;
    (void)port;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=ServerX/1.0");
    env.push_back("SERVER_PROTOCOL=HTTP/1.1");

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
    std::string interpreter_path = req.location()->get("cgi_pass", req)[0];
    std::string script_path = req.resolvePath();
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
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        close(pipe_fd[1]);
        char *argv[] = {const_cast<char *>(interpreter_path.c_str()), // argv[0] = /usr/bin/python3
                        const_cast<char *>(script_path.c_str()), // argv[1] = /var/www/script.py
                        NULL};

        // Build CGI environment
        std::vector<std::string> env = buildCgiEnvironment(req, 0);
        char**  envp = vectorToCharArray(env);
        execve(interpreter_path.c_str(), argv, envp);
        LOG_ERROR("EXECVE")
        exit(127);
    }
    if (pid) {
        close(pipe_fd[1]);
        res.setBodyFromCgi(pipe_fd[0]);
    }
};

} // namespace http
