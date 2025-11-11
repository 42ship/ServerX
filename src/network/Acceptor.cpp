#include "network/Acceptor.hpp"

#include "config/ServerBlock.hpp"
#include "http/Router.hpp"
#include "network/ClientHandler.hpp"
#include "network/EventDispatcher.hpp"
#include "network/Socket.hpp"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>

namespace {
// Helper function to convert sockaddr_in to IP string
std::string socketAddrToString(struct sockaddr_in const &addr) {
    char ipStr[INET_ADDRSTRLEN];  // 16 bytes: "xxx.xxx.xxx.xxx\0"

    // inet_ntop is preferred (thread-safe, IPv6 ready)
    if (inet_ntop(AF_INET, &addr.sin_addr, ipStr, INET_ADDRSTRLEN)) {
        return std::string(ipStr);
    }

    // Fallback: use inet_ntoa (C++98 compatible but less safe)
    return std::string(inet_ntoa(addr.sin_addr));
}
}

namespace network {

Acceptor::Acceptor(int port, http::Router const &router)
    : socket_(port), port_(port), router_(router) {
    if (listen(socket_.getFd(), 10) < 0) {
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
}

Acceptor::Acceptor(config::ServerBlock const &s, http::Router const &router)
    : socket_(s), router_(router) {
    if (listen(socket_.getFd(), 10) < 0) {
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
    port_ = s.port();
}

Acceptor::~Acceptor() {}

void Acceptor::handleEvent(uint32_t events) {
    if (events & EPOLLIN) {
        acceptNewConnection();
    }
}

int Acceptor::getFd() const { return socket_.getFd(); }

void Acceptor::acceptNewConnection() {
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int clientFd = accept(socket_.getFd(), reinterpret_cast<sockaddr *>(&clientaddr), &len);
    if (clientFd < 0) {
        std::cerr << "Accept error: " << strerror(errno) << std::endl;
        return;
    }

    // Convert client address to string
    std::string clientAddrStr = socketAddrToString(clientaddr);

    // Set non-blocking
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

    // Pass client address to ClientHandler
    ClientHandler *clientHandler = new ClientHandler(clientFd, port_, clientAddrStr, router_);
    EventDispatcher::getInstance().registerHandler(clientHandler);
}

} // namespace network
