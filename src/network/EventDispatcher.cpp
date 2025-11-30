#include "network/EventDispatcher.hpp"

#include "network/IEventHandler.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>
#include <exception>

#define MAX_EVENTS 1024

namespace network {

namespace {
bool checkHandler(IEventHandler const *handler) { return handler && handler->getFd() >= 0; }

inline bool deleteHandlerSafely(IEventHandler *handler) {
    try {
        delete handler;
    } catch (std::exception const &e) {
        LOG_SERROR(e.what());
        return false;
    } catch (...) {
        LOG_SERROR("Unknown error");
        return false;
    }
    return true;
}

} // namespace

EventDispatcher::EventDispatcher() {}

EventDispatcher::~EventDispatcher() {
    for (std::map<int, IEventHandler *>::iterator it = handlers_.begin(); it != handlers_.end();
         ++it) {
        deleteHandlerSafely(it->second);
    }
    handlers_.clear();
    cleanUpGarbage();
}

EventDispatcher &EventDispatcher::getInstance() {
    static EventDispatcher instance;
    return instance;
}

void EventDispatcher::registerHandler(IEventHandler *handler) {
    if (!checkHandler(handler))
        return;
    activeHandlers_.insert(handler);
    handlers_[handler->getFd()] = handler;
    handler->registeredEvents_ = EPOLLIN;
    epollManager_.addHandler(handler, EPOLLIN);
}

void EventDispatcher::removeHandler(IEventHandler *handler) {
    if (!handler)
        return;
    activeHandlers_.erase(handler);
    if (handler->getFd() >= 0) {
        epollManager_.modifyHandler(handler, 0);
        handlers_.erase(handler->getFd());
    }
    pendingRemovals_.insert(handler);
}

void EventDispatcher::cleanUpGarbage() {
    for (std::set<IEventHandler *>::iterator it = pendingRemovals_.begin();
         it != pendingRemovals_.end(); ++it) {
        deleteHandlerSafely(*it);
    }
    pendingRemovals_.clear();
}

void EventDispatcher::requestShutdown() { epollManager_.requestShutdown(); }

void EventDispatcher::updateEventMask(IEventHandler *handler, uint32_t newMask) {
    if (handler->registeredEvents_ == newMask) {
        return;
    }
    epollManager_.modifyHandler(handler, newMask);
    handler->registeredEvents_ = newMask;
}

void EventDispatcher::enableRead(IEventHandler *handler) {
    if (!handler)
        return;
    updateEventMask(handler, handler->registeredEvents_ | EPOLLIN);
}
void EventDispatcher::enableWrite(IEventHandler *handler) {
    if (!handler)
        return;
    updateEventMask(handler, handler->registeredEvents_ | EPOLLOUT);
}
void EventDispatcher::disableRead(IEventHandler *handler) {
    if (!handler)
        return;
    updateEventMask(handler, handler->registeredEvents_ & ~EPOLLIN);
}
void EventDispatcher::disableWrite(IEventHandler *handler) {
    if (!handler)
        return;
    updateEventMask(handler, handler->registeredEvents_ & ~EPOLLOUT);
}

// TODO will need to implement shutdown inside else if nready == 0
void EventDispatcher::handleEvents() {
    static bool was_printed = false;
    while (!epollManager_.getisShuttingDown()) {
        struct epoll_event events[MAX_EVENTS];
        int nready = epollManager_.waitForEvents(events, MAX_EVENTS, 1000);
        if (nready < 0) {
            cleanUpGarbage();
            if (errno == EINTR) {
                LOG_DEBUG("epoll_wait interrupted by a signal, continuing...");
                continue;
            }
            LOG_ERROR("epoll_wait failed: " << strerror(errno));
            break;
        } else if (nready == 0) {
            cleanUpGarbage();
            if (was_printed)
                continue;
            LOG_DEBUG("epoll_wait timed out, no events to process.");
            was_printed = true;
            continue;
        }
        was_printed = false;
        for (int i = 0; i < nready; ++i) {
            IEventHandler *handler = static_cast<IEventHandler *>(events[i].data.ptr);
            // SAFETY CHECK: remove handler was called from previos handler for this handler
            if (activeHandlers_.find(handler) == activeHandlers_.end())
                continue;
            try {
                handler->handleEvent(events[i].events);
            } catch (std::exception const &e) {
                LOG_ERROR("EventDispatcher::handleEvents::handler("
                          << handler->getFd() << ")->handleEvent: " << e.what());
                removeHandler(handler);
            } catch (...) {
                LOG_ERROR("EventDispatcher::handleEvents::handler("
                          << handler->getFd() << ")->handleEvent: unknown error");
                removeHandler(handler);
            }
        }
        cleanUpGarbage();
    }
}

} // namespace network
