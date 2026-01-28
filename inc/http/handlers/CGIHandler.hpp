#pragma once

#include "http/Handler.hpp"
#include <string>
#include <vector>

namespace http {

class CGIHandler : public IHandler {
public:
    static std::string formatHeaderName(std::string const &name);

    static void handle(Request const &, Response &, MimeTypes const &);

private:
    CGIHandler(Request const &req, Response &res);

    void handle();
    void buildArgv();
    void buildEnvp();
    bool initPipes();
    void fork();
    int execve();
    void runChildProcess();
    void runParentProcess();

    Request const &req_;
    Response &res_;

    std::vector<std::string> argv_;
    std::vector<std::string> envp_;
    int pipeFd_[2];
    int errorFd_[2];
    int pid_;
};

} // namespace http
