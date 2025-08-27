#pragma once

#include "EpollManager.hpp"
#include "AEventHandler.hpp"
#include <map>

namespace network {

/**
 * @brief Centralised event demultiplexer and dispatcher for managing I/O events.
 *
 * InitiationDispatcher coordinates between the event detection mechanism (epoll)
 * and register event handlers. It maintains a registry of handlers, waits for events
 * to occur on monitored file descriptors, and dispatches events to appropriate handlers.
 * This implements the core of the Reactor pattern for non-blocking, event-driven I/O.
 *
 */
class InitiationDispatcher {
public:
    static InitiationDispatcher &getInstance();

    void registerHandler(AEventHandler *handler);
    void removeHandler(int fd);
    void handleEvents();
    void requestShutdown();
    EpollManager &getEpollManager();

private:
    InitiationDispatcher();
    ~InitiationDispatcher();

    EpollManager epollManager_;
    std::map<int, AEventHandler *> handlers_;
    static InitiationDispatcher *instance_;

    InitiationDispatcher(const InitiationDispatcher &);
    InitiationDispatcher &operator=(const InitiationDispatcher &);
};

} // namespace network
