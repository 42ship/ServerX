#include "network/InitiationDispatcher.hpp"

#include <iostream>
#include "network/AEventHandler.hpp"

namespace network {

InitiationDispatcher *InitiationDispatcher::instance_ = NULL;

InitiationDispatcher &InitiationDispatcher::getInstance() {
    if (instance_ == NULL) {
        instance_ = new InitiationDispatcher();
    }
    return *instance_;
}

InitiationDispatcher::InitiationDispatcher() {
    std::cout << "InitiationDispatcher created" << std::endl;
}

InitiationDispatcher::~InitiationDispatcher() {
    for (std::map<int, AEventHandler *>::iterator it = handlers_.begin(); it != handlers_.end();
         ++it) {
        delete it->second;
    }
    handlers_.clear();
}

void InitiationDispatcher::registerHandler(AEventHandler *handler) {
    int fd = handler->getHandle();
    handlers_[fd] = handler;
    epollManager_.addFd(fd, EPOLLIN);
}

void InitiationDispatcher::removeHandler(int fd) {
    std::map<int, AEventHandler *>::iterator it = handlers_.find(fd);
    if (it != handlers_.end()) {
        epollManager_.removeFd(fd);
        handlers_.erase(it);
    }
}

// TODO will need to implement shutdown inside else if nready == 0
void InitiationDispatcher::handleEvents() {
    std::cout << "Starting event loop (signal-aware)" << std::endl;
    while (!epollManager_.getisShuttingDown()) {
        struct epoll_event events[1024];
        int nready = epollManager_.waitForEvents(events, 1024, 1000);
        if (nready < 0) {
            std::cerr << "epoll_wait error" << std::endl;
            break;
        } else if (nready == 0) {
            // implement here
            break;
        }
        for (int i = 0; i < nready; ++i) {
            int fd = events[i].data.fd;

            std::map<int, AEventHandler *>::iterator it = handlers_.find(fd);
            if (it != handlers_.end()) {
                it->second->handleEvent(events[i].events);
            }
        }
    }
    std::cout << "Event loop exited" << std::endl;
}

void InitiationDispatcher::requestShutdown() {
    epollManager_.requestShutdown();
}

EpollManager &InitiationDispatcher::getEpollManager() {
    return epollManager_;
}

} // namespace network
