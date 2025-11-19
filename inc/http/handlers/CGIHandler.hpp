#pragma once

#include "http/Handler.hpp"

namespace http {

class CGIHandler : public IHandler {
public:
    static std::string formatHeaderName(std::string const &name);

    static void handle(Request const &, Response &);

private:
    CGIHandler(Request const &req, Response &res);

    void handle();
    bool buildArgv();
    bool buildEnvp();
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

    bool hasHeaderParsing_;
};

} // namespace http
