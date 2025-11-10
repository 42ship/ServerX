#pragma once

#include "EpollManager.hpp"
#include "IEventHandler.hpp"
#include <map>

namespace network {

/**
 * @brief Centralised event demultiplexer and dispatcher for managing I/O events.
 *
 * EventDispatcher coordinates between the event detection mechanism (epoll)
 * and register event handlers. It maintains a registry of handlers, waits for events
 * to occur on monitored file descriptors, and dispatches events to appropriate handlers.
 * This implements the core of the ClientHandler pattern for non-blocking, event-driven I/O.
 *
 */
class EventDispatcher {
public:
    ~EventDispatcher();

    void registerHandler(IEventHandler *handler);
    void modifyHandler(IEventHandler *handler, uint32_t events);
    void removeHandler(IEventHandler *handler);
    void setSendingData(IEventHandler *handler);
    void setReceivingData(IEventHandler *handler);
    void handleEvents();
    void requestShutdown();

    static EventDispatcher &getInstance();

private:
    EventDispatcher();
    EventDispatcher(const EventDispatcher &);
    EventDispatcher &operator=(const EventDispatcher &);

    EpollManager epollManager_;
    std::map<int, IEventHandler *> handlers_;
};

} // namespace network
