#include "http/handlers/CGIHandler.hpp"
#include "common/error.hpp"
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

namespace {

bool isNPH(const std::string &path) {
    size_t lastSlash = path.rfind('/');
    size_t startPos = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    if (path.length() - startPos < 3) {
        return false;
    }
    return path.compare(startPos, 3, "nph") == 0;
}

bool isNPH(Request const &req) { return req.location()->has("cgi_nph") || isNPH(req.path()); }

} // namespace

static std::vector<char *> toCStringVector(std::vector<std::string> const &v);

CGIHandler::CGIHandler(Request const &req, Response &res)
    : req_(req), res_(res), pid_(-1), hasHeaderParsing_(false) {
    argv_.reserve(3);
    envp_.reserve(20);
}

void CGIHandler::handle(Request const &req, Response &res) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    CGIHandler handler(req, res);
    handler.handle();
}

void CGIHandler::handle() {
    if (!buildArgv() || !buildEnvp() || !initPipes())
        return;
    hasHeaderParsing_ = !isNPH(req_);
    this->fork();
    switch (pid_) {
    case -1:
        return;
    case 0:
        return runChildProcess();
    default:
        return runParentProcess();
    }
}

void CGIHandler::runChildProcess() {
    close(errorFd_[0]);
    close(pipeFd_[0]);
    dup2(pipeFd_[1], STDOUT_FILENO);
    close(pipeFd_[1]);
    if (req_.body() >= 0) {
        dup2(req_.body(), STDIN_FILENO);
    } else {
        dup2(core::Server::getNullFd(), STDIN_FILENO);
    }
    // dup2(errorFd_[1], STDERR_FILENO);
    execve();
    print_errno_to_fd(errorFd_[1], errno);
    close(errorFd_[1]);
    _exit(127);
}

void CGIHandler::runParentProcess() {
    close(pipeFd_[1]);
    close(errorFd_[1]);
    char buf[257];
    size_t bread = read(errorFd_[0], buf, sizeof(buf) - 1);
    buf[bread] = 0;
    close(errorFd_[0]);
    if (bread > 0) {
        close(pipeFd_[0]);
        LOG_ERROR("CGIHandler::handle(" << req_.path() << ")::execve: " << buf);
        return (void)res_.status(INTERNAL_SERVER_ERROR);
    } else {
        res_.setBodyFromCgi(pipeFd_[0], hasHeaderParsing_);
    }
}

bool CGIHandler::initPipes() {
    if (pipe(pipeFd_)) {
        LOG_ERROR("CGIHandler::pipe: " << strerror(errno));
        return res_.status(INTERNAL_SERVER_ERROR), false;
    }
    if (pipe2(errorFd_, O_CLOEXEC)) {
        close(pipeFd_[0]);
        close(pipeFd_[1]);
        LOG_ERROR("CGIHandler::pipe2: " << strerror(errno));
        return res_.status(INTERNAL_SERVER_ERROR), false;
    }
    return true;
}

void CGIHandler::fork() {
    pid_ = ::fork();
    if (pid_ != -1)
        return;
    close(pipeFd_[0]);
    close(pipeFd_[1]);
    close(errorFd_[0]);
    close(errorFd_[1]);
    LOG_ERROR("CGIHandler::fork: " << strerror(errno));
    res_.status(INTERNAL_SERVER_ERROR);
}

bool CGIHandler::buildArgv() {
    std::string script_path = req_.resolvePath();
    if (req_.location()->has("cgi_pass")) {
        std::vector<std::string> const &cgi_pass = req_.location()->get("cgi_pass", req_);
        if (!cgi_pass.empty()) {
            argv_.push_back(cgi_pass[0]);
            argv_.push_back(script_path);
        } else
            argv_.push_back(script_path);
    } else
        argv_.push_back(script_path);
    return true;
}

bool CGIHandler::buildEnvp() {
    // CGI/1.1 Meta-Variables (RFC 3875)
    envp_.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envp_.push_back("SERVER_SOFTWARE=ServerX/1.0");
    envp_.push_back("SERVER_PROTOCOL=" + req_.version());

    envp_.push_back("REQUEST_METHOD=" +
                    std::string(RequestStartLine::methodToString(req_.method())));
    envp_.push_back("SCRIPT_NAME=" + req_.path());
    envp_.push_back("QUERY_STRING=" + req_.queryString());

    envp_.push_back("SERVER_PORT=" + utils::toString(req_.server()->port()));
    envp_.push_back("REMOTE_ADDR=" + req.remoteAddr());

    std::string contentType = req.headers().get("Content-Type");
    if (!contentType.empty()) {
        env.push_back("CONTENT_TYPE=" + contentType);
    }

    std::string contentLength = req.headers().get("Content-Length");
    if (!contentLength.empty()) {
        env.push_back("CONTENT_LENGTH=" + contentLength);
    }

    for (Headers::HeaderMap::const_iterator it = req_.headers().begin(); it != req_.headers().end();
         ++it) {
        envp_.push_back(formatHeaderName(it->first) + "=" + it->second);
    }
    return true;
}

std::string CGIHandler::formatHeaderName(const std::string &name) {
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

static std::vector<char *> toCStringVector(std::vector<std::string> const &v) {
    std::vector<char *> res;
    res.reserve(v.size() + 1);
    for (size_t i = 0; i < v.size(); i++) {
        res.push_back(const_cast<char *>(v[i].c_str()));
    }
    res.push_back(NULL);
    return res;
}

int CGIHandler::execve() {
    std::vector<char *> const &argv_ptrs = toCStringVector(argv_);
    std::vector<char *> const &envp_ptrs = toCStringVector(envp_);
    return ::execve(argv_ptrs[0], &argv_ptrs[0], &envp_ptrs[0]);
}

} // namespace http
