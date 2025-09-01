#include "network/Acceptor.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include "network/InitiationDispatcher.hpp"
#include "network/Reactor.hpp"
#include "core/Socket.hpp"
#include "config/ServerBlock.hpp"

namespace network {

Acceptor::Acceptor(int port) : socket_(port) {
    if (listen(socket_.getFd(), 10) < 0) {
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
    std::cout << "Listening on port " << port << std::endl;
}

Acceptor::Acceptor(config::ServerBlock const &s) : socket_(s) {
    if (listen(socket_.getFd(), 10) < 0) {
        throw std::runtime_error("Failed to listen on socket: " + std::string(strerror(errno)));
    }
    std::cout << "Listening on port " << s.getPort() << std::endl;
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

    int clientFd = accept(socket_.getFd(), (struct sockaddr *)&clientaddr, &len);
    if (clientFd < 0) {
        std::cerr << "Accept error: " << strerror(errno) << std::endl;
        return;
    }
    std::cout << "Accept finished: " << clientFd << std::endl;

    Reactor *clientHandler = new Reactor(clientFd);
    InitiationDispatcher::getInstance().registerHandler(clientHandler);
}

} // namespace network
