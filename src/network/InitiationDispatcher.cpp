#include "network/InitiationDispatcher.hpp"

#include "network/AEventHandler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>

namespace network {

InitiationDispatcher &InitiationDispatcher::getInstance() {
    static InitiationDispatcher instance;
    return instance;
}

InitiationDispatcher::InitiationDispatcher() {}

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
    if (fd < 0)
        return;
    std::map<int, AEventHandler *>::iterator it = handlers_.find(fd);
    if (it != handlers_.end()) {
        epollManager_.removeFd(fd);
        delete it->second;
        handlers_.erase(it);
    }
}

// TODO will need to implement shutdown inside else if nready == 0
void InitiationDispatcher::handleEvents() {
    static bool wasPrinted = false;
    LOG_INFO("Starting event loop (signal-aware)");
    while (!epollManager_.getisShuttingDown()) {
        struct epoll_event events[1024];
        int nready = epollManager_.waitForEvents(events, 1024, 1000);
        if (nready < 0) {
            if (errno == EINTR) {
                LOG_DEBUG("epoll_wait interrupted by a signal, continuing...");
                continue;
            }
            LOG_ERROR("epoll_wait failed: " << strerror(errno));
            break;
        } else if (nready == 0) {
            if (!wasPrinted) {
                LOG_DEBUG("epoll_wait timed out, no events to process.");
                wasPrinted = true;
            }
            continue;
        }
        for (int i = 0; i < nready; ++i) {
            int fd = events[i].data.fd;

            std::map<int, AEventHandler *>::iterator it = handlers_.find(fd);
            if (it != handlers_.end()) {
                wasPrinted = false;
                it->second->handleEvent(events[i].events);
            } else {
                LOG_WARN("Received event for untracked fd: "
                         << fd << ". It may have been recently closed.");
                epollManager_.removeFd(fd);
            }
        }
    }
    LOG_INFO("Event loop exited");
}

void InitiationDispatcher::requestShutdown() { epollManager_.requestShutdown(); }

EpollManager &InitiationDispatcher::getEpollManager() { return epollManager_; }

} // namespace network
