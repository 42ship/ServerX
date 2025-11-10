#include "config/LocationBlock.hpp"
#include "http/Handler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>

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
        execve(interpreter_path.c_str(), argv, NULL);
        LOG_ERROR("EXECVE")
        exit(127);
    }
    if (pid) {
        close(pipe_fd[1]);
        res.setBodyFromCgi(pipe_fd[0]);
    }
};

} // namespace http
