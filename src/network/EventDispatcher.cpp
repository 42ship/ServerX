#include "network/EventDispatcher.hpp"

#include "network/IEventHandler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>

#define MAX_EVENTS 1024

namespace network {

namespace {
bool checkHandler(IEventHandler *handler) { return handler && handler->getFd() >= 0; }
} // namespace

EventDispatcher::EventDispatcher() {}

EventDispatcher::~EventDispatcher() {
    for (std::map<int, IEventHandler *>::iterator it = handlers_.begin(); it != handlers_.end();
         ++it) {
        delete it->second;
    }
    handlers_.clear();
}

EventDispatcher &EventDispatcher::getInstance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::registerHandler(IEventHandler *handler) {
    if (!checkHandler(handler))
        return;
    handlers_[handler->getFd()] = handler;
    epollManager_.addHandler(handler, EPOLLIN);
}

void EventDispatcher::removeHandler(IEventHandler *handler) {
    if (!checkHandler(handler))
        return;
    std::map<int, IEventHandler *>::iterator it = handlers_.find(handler->getFd());
    if (it != handlers_.end()) {
        epollManager_.removeHandler(it->second);
        delete it->second;
        handlers_.erase(it);
    }
}

void EventDispatcher::modifyHandler(IEventHandler *handler, uint32_t events) {
    if (checkHandler(handler))
        epollManager_.modifyHandler(handler, events);
}
void EventDispatcher::setSendingData(IEventHandler *handler) { modifyHandler(handler, EPOLLOUT); }
void EventDispatcher::setReceivingData(IEventHandler *handler) { modifyHandler(handler, EPOLLIN); }
void EventDispatcher::requestShutdown() { epollManager_.requestShutdown(); }

// TODO will need to implement shutdown inside else if nready == 0
void EventDispatcher::handleEvents() {
    static bool was_printed = false;
    while (!epollManager_.getisShuttingDown()) {
        struct epoll_event events[MAX_EVENTS];
        int nready = epollManager_.waitForEvents(events, MAX_EVENTS, 1000);
        if (nready < 0) {
            if (errno == EINTR) {
                LOG_DEBUG("epoll_wait interrupted by a signal, continuing...");
                continue;
            }
            LOG_ERROR("epoll_wait failed: " << strerror(errno));
            break;
        } else if (nready == 0) {
            if (was_printed)
                continue;
            LOG_DEBUG("epoll_wait timed out, no events to process.");
            was_printed = true;
            continue;
        }
        was_printed = false;
        for (int i = 0; i < nready; ++i) {
            IEventHandler *handler = static_cast<IEventHandler *>(events[i].data.ptr);
            handler->handleEvent(events[i].events);
        }
    }
}

} // namespace network
