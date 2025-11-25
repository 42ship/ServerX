#include "network/EpollManager.hpp"
#include "network/IEventHandler.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>

namespace network {

EpollManager::EpollManager() : epollFd_(-1), isShuttingDown(0) {
    epollFd_ = epoll_create1(0);
    if (epollFd_ < 0) {
        throw std::runtime_error("Failed to create epoll instance: " +
                                 std::string(strerror(errno)));
    }
}

EpollManager::~EpollManager() {
    if (epollFd_ >= 0) {
        close(epollFd_);
    }
}

void EpollManager::addHandler(IEventHandler *handler, uint32_t events) {
    if (!handler)
        return;
    struct epoll_event eventStruct;
    eventStruct.events = events;
    eventStruct.data.ptr = handler;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, handler->getFd(), &eventStruct) < 0) {
        throw std::runtime_error("Failed to add fd to epoll: " + std::string(strerror(errno)));
    }
}

void EpollManager::modifyHandler(IEventHandler *handler, uint32_t events) {
    if (!handler)
        return;
    struct epoll_event eventStruct;
    eventStruct.events = events;
    eventStruct.data.ptr = handler;
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, handler->getFd(), &eventStruct) < 0) {
        throw std::runtime_error("Failed to add fd to epoll: " + std::string(strerror(errno)));
    }
}

void EpollManager::removeHandler(IEventHandler const *handler) {
    if (!handler)
        return;
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, handler->getFd(), NULL) < 0) {
        throw std::runtime_error("Failed to remove fd to epoll: " + std::string(strerror(errno)));
    }
}

// TODO research on the industry standard what the best timeout should I set up for is
int EpollManager::waitForEvents(struct epoll_event *events, int maxEvents, int timeout) {
    return epoll_wait(epollFd_, events, maxEvents, timeout);
}

void EpollManager::requestShutdown() { isShuttingDown = 1; }

bool EpollManager::getisShuttingDown() const { return isShuttingDown != 0; }

} // namespace network
