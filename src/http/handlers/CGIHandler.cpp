#include "http/handlers/CGIHandler.hpp"
#include "common/error.hpp"
#include "common/filesystem.hpp"
#include "common/string.hpp"
#include "config/LocationBlock.hpp"
#include "core/Server.hpp"
#include "http/Handler.hpp"
#include "http/HttpStatus.hpp"
#include "http/handlers/DirectoryListingHandler.hpp"
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

bool isValidCgiHeaderName(std::string const &headerName) {
    if (headerName.empty()) {
        return false;
    }
    for (size_t i = 0; i < headerName.length(); ++i) {
        char c = headerName[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '-' || c == '_' || c == '.')) {
            return false;
        }
    }
    return true;
}

} // namespace

CGIHandler::CGIHandler(Request const &req, Response &res) : req_(req), res_(res), pid_(-1) {
    pipeFd_[0] = -1;
    pipeFd_[1] = -1;
    errorFd_[0] = -1;
    errorFd_[1] = -1;
    argv_.reserve(3);
    envp_.reserve(30); // Increased reservation for more env vars
}

void CGIHandler::handle(Request const &req, Response &res, MimeTypes const &mime) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);

    std::string path = req.resolvePath();
    struct stat st;

    if (!utils::getFileStatus(path, st)) {
        res.status(NOT_FOUND);
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        if (req.location()->autoIndex()) {
            return DirectoryListingHandler::handle(req, res, mime);
        }
        return StaticFileHandler::handle(req, res, mime);
    }

    if (!(st.st_mode & S_IXUSR)) {
        res.status(FORBIDDEN);
        return;
    }

    CGIHandler handler(req, res);
    handler.handle();
}

void CGIHandler::handle() {
    if (!initPipes()) {
        res_.status(INTERNAL_SERVER_ERROR);
        return;
    }
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
    buildArgv();
    buildEnvp();

    close(errorFd_[0]);
    close(pipeFd_[0]);

    if (dup2(pipeFd_[1], STDOUT_FILENO) == -1) {
        print_errno_to_fd(errorFd_[1], errno);
        _exit(1);
    }
    close(pipeFd_[1]);

    if (req_.body() >= 0) {
        if (dup2(req_.body(), STDIN_FILENO) == -1) {
            print_errno_to_fd(errorFd_[1], errno);
            _exit(1);
        }
    } else {
        if (dup2(core::Server::getNullFd(), STDIN_FILENO) == -1) {
            print_errno_to_fd(errorFd_[1], errno);
            _exit(1);
        }
    }

    execve();
    print_errno_to_fd(errorFd_[1], errno);
    close(errorFd_[1]);
    _exit(127);
}

void CGIHandler::runParentProcess() {
    close(pipeFd_[1]);
    close(errorFd_[1]);
    char buf[257];
    ssize_t bytesRead = read(errorFd_[0], buf, sizeof(buf) - 1);
    buf[bytesRead] = 0;
    close(errorFd_[0]);

    if (bytesRead < 0) {
        close(pipeFd_[0]);
        LOG_SERROR("read: " << strerror(errno));
        return (void)res_.status(INTERNAL_SERVER_ERROR);
    } else if (bytesRead > 0) {
        close(pipeFd_[0]);
        LOG_SERROR("execve: " << buf);
        return (void)res_.status(INTERNAL_SERVER_ERROR);
    } else {
        res_.setBodyFromCgi(pipeFd_[0], !isNPH(req_));
    }
}

bool CGIHandler::initPipes() {
    if (pipe(pipeFd_)) {
        LOG_SERROR(strerror(errno));
        return false;
    }
    if (pipe2(errorFd_, O_CLOEXEC)) {
        close(pipeFd_[0]);
        close(pipeFd_[1]);
        LOG_SERROR(strerror(errno));
        return false;
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
    LOG_SERROR(strerror(errno));
    res_.status(INTERNAL_SERVER_ERROR);
}

void CGIHandler::buildArgv() {
    std::string script_path = req_.resolvePath();
    if (req_.location()->has("cgi_pass")) {
        std::vector<std::string> const &cgi_pass = req_.location()->getRawValues("cgi_pass");
        if (cgi_pass.empty() || (cgi_pass.size() == 1 && cgi_pass[0] == "enabled")) {
            // Case 0: enabled but no interpreter specified
            argv_.push_back(script_path);
        } else if (cgi_pass.size() == 1) {
            // Case 1: single interpreter specified for all files
            argv_.push_back(cgi_pass[0]);
            argv_.push_back(script_path);
        } else {
            // Case 2: extension mapping (extension, interpreter)
            std::string ext = utils::getFileExtension(script_path);
            if (!ext.empty() && ext[0] != '.') {
                ext = "." + ext;
            }
            if (!ext.empty() && ext == cgi_pass[0]) {
                argv_.push_back(cgi_pass[1]);
                argv_.push_back(script_path);
            } else {
                argv_.push_back(script_path);
            }
        }
    } else {
        argv_.push_back(script_path);
    }
}

void CGIHandler::buildEnvp() {
    envp_.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envp_.push_back("SERVER_SOFTWARE=ServerX/1.0");
    envp_.push_back("SERVER_PROTOCOL=" + req_.version());

    envp_.push_back("REQUEST_METHOD=" +
                    std::string(RequestStartLine::methodToString(req_.method())));
    envp_.push_back("SCRIPT_NAME=" + req_.path());
    envp_.push_back("QUERY_STRING=" + req_.queryString());

    envp_.push_back("PATH_INFO=");
    envp_.push_back("PATH_TRANSLATED=");

    envp_.push_back("SERVER_PORT=" + utils::toString(req_.server()->port()));

    if (req_.server()->has("server_name")) {
        std::vector<std::string> serverNames = req_.server()->get("server_name", req_);
        if (!serverNames.empty()) {
            envp_.push_back("SERVER_NAME=" + serverNames[0]);
        }
    }

    envp_.push_back("REMOTE_ADDR=" + req_.remoteAddr());

    std::string contentType = req_.headers().get("Content-Type");
    if (!contentType.empty()) {
        envp_.push_back("CONTENT_TYPE=" + contentType);
    }

    std::string contentLength = req_.headers().get("Content-Length");
    if (!contentLength.empty()) {
        envp_.push_back("CONTENT_LENGTH=" + contentLength);
    }

    for (Headers::HeaderMap::const_iterator it = req_.headers().begin(); it != req_.headers().end();
         ++it) {

        std::string headerName = it->first;

        if (!isValidCgiHeaderName(headerName))
            continue;

        if (headerName == "content-type" || headerName == "content-length")
            continue;

        envp_.push_back(formatHeaderName(headerName) + "=" + it->second);
    }
}

std::string CGIHandler::formatHeaderName(const std::string &name) {
    std::string cgi_name = "HTTP_";
    for (size_t i = 0; i < name.length(); ++i) {
        char c = name[i];
        if (c == '-') {
            cgi_name += '_';
        } else {
            cgi_name += std::toupper(static_cast<unsigned char>(c));
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
