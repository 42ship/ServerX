#include "network/EpollManager.hpp"

#include <sys/epoll.h>
#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cerrno>
#include <cstdio>
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

void EpollManager::addFd(int fd, uint32_t events) {
    struct epoll_event eventStruct;
    eventStruct.events = events;
    eventStruct.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &eventStruct) < 0) {
        throw std::runtime_error("Failed to add fd to epoll: " + std::string(strerror(errno)));
    }
}

void EpollManager::removeFd(int fd) {
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
        throw std::runtime_error("Failed to remove fd to epoll: " + std::string(strerror(errno)));
    }
}

void EpollManager::modifyFd(int fd, uint32_t events) {
    struct epoll_event eventStruct;
    eventStruct.events = events;
    eventStruct.data.fd = fd;

    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &eventStruct) < 0) {
        throw std::runtime_error("Failed to modify fd in epoll: " + std::string(strerror(errno)));
    }
}

// TODO research on the industry standard what the best timeout should I set up for is
int EpollManager::waitForEvents(struct epoll_event *events, int maxEvents, int timeout) {
    while (!isShuttingDown) {
        int result = epoll_wait(epollFd_, events, maxEvents, timeout);

        if (result > 0) {
            return result;
        } else if (result == 0) {
            // need to set up a timeout for shutdown flag and continue
            continue;
        } else if (errno == EINTR) {
            std::cout << "epoll_wait interrupted by signal" << std::endl;
            continue;
        } else {
            return result;
        }
    }
    std::cout << "Shutdown requested, exiting event loop" << std::endl;
    return 0;
}

void EpollManager::requestShutdown() {
    isShuttingDown = 1;
}

bool EpollManager::getisShuttingDown() const {
    return isShuttingDown != 0;
}

} // namespace network
