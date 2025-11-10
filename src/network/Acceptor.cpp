#include "network/Acceptor.hpp"

#include "config/ServerBlock.hpp"
#include "http/Router.hpp"
#include "network/ClientHandler.hpp"
#include "network/EventDispatcher.hpp"
#include "network/Socket.hpp"
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>

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
    int flags = fcntl(clientFd, F_GETFL, 0);
    fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);
    ClientHandler *clientHandler = new ClientHandler(clientFd, port_, router_);
    EventDispatcher::getInstance().registerHandler(clientHandler);
}

} // namespace network
