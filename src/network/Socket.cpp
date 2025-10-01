#include "network/Socket.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

namespace network {

Socket::Socket(void) : fd_(-1) { std::memset(&addr_, 0, sizeof(addr_)); }

Socket::Socket(int port) : fd_(-1) { createAndBind("0.0.0.0", port); }

Socket::Socket(std::string const &address, int port) : fd_(-1) { createAndBind(address, port); }

Socket::Socket(config::ServerBlock const &s) : fd_(-1) {
    if (s.getAddress().empty()) {
        createAndBind("0.0.0.0", s.getPort());
    } else
        createAndBind(s.getAddress(), s.getPort());
}

Socket::~Socket(void) {
    if (fd_ > 0)
        close(fd_);
}

int Socket::getFd(void) const { return fd_; }

const struct sockaddr_in &Socket::getAddr(void) const { return addr_; }

void Socket::bindTo(int port) { bindTo("0.0.0.0", port); }

void Socket::bindTo(std::string const &address, int port) {
    if (fd_ >= 0) {
        throw std::runtime_error("Socket is already bound");
    }
    createAndBind(address, port);
}

void Socket::createAndBind(std::string const &address, int port) {
    if (port < 0 || port > 65535) {
        throw std::runtime_error("Invalid port number. Port must be between 1024 and 65535");
    }
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ < 0)
        throw std::runtime_error("Failed to create socket: " + std::string(strerror(errno)));
    int opt = 1;
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(fd_);
        fd_ = -1;
        throw std::runtime_error("Failed to set socket options: " + std::string(strerror(errno)));
    }
    std::memset(&addr_, 0, sizeof(addr_));
    addr_.sin_port = htons(port);
    addr_.sin_family = AF_INET;
    if (inet_pton(AF_INET, address.c_str(), &addr_.sin_addr.s_addr) <= 0) {
        close(fd_);
        fd_ = -1;
        throw std::runtime_error("Invalid IP address");
    }
    if (bind(fd_, reinterpret_cast<sockaddr *>(&addr_), sizeof(addr_)) < 0) {
        close(fd_);
        fd_ = -1;
        throw std::runtime_error("Failed to bind socket: " + std::string(strerror(errno)));
    }
}

} // namespace network
