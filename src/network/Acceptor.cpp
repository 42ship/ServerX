#include "network/Acceptor.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include "http/Router.hpp"
#include "network/InitiationDispatcher.hpp"
#include "network/Reactor.hpp"
#include "network/Socket.hpp"
#include "config/ServerBlock.hpp"

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
    port_ = s.getPort();
}

Acceptor::~Acceptor() {
}

void Acceptor::handleEvent(uint32_t events) {
    if (events & EPOLLIN) {
        acceptNewConnection();
    }
}

int Acceptor::getHandle() const {
    return socket_.getFd();
}

void Acceptor::acceptNewConnection() {
    struct sockaddr_in clientaddr;
    socklen_t len = sizeof(clientaddr);

    int clientFd = accept(socket_.getFd(), reinterpret_cast<sockaddr *>(&clientaddr), &len);
    if (clientFd < 0) {
        std::cerr << "Accept error: " << strerror(errno) << std::endl;
        return;
    }
    Reactor *clientHandler = new Reactor(clientFd, port_, router_);
    InitiationDispatcher::getInstance().registerHandler(clientHandler);
}

} // namespace network
