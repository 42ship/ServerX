#pragma once

#include "EpollManager.hpp"
#include "IEventHandler.hpp"
#include <map>
#include <set>

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
    void removeHandler(IEventHandler *handler);
    void handleEvents();
    void requestShutdown();

    static EventDispatcher &getInstance();

public:
    /**
     * @brief Adds EPOLLIN to the monitored events.
     * @note No-op if already reading.
     */
    void enableRead(IEventHandler *handler);

    /**
     * @brief Removes EPOLLIN from the monitored events.
     */
    void disableRead(IEventHandler *handler);

    /**
     * @brief Adds EPOLLOUT to the monitored events.
     * @note No-op if already writing.
     */
    void enableWrite(IEventHandler *handler);

    /**
     * @brief Removes EPOLLOUT from the monitored events.
     */
    void disableWrite(IEventHandler *handler);

private:
    void updateEventMask(IEventHandler *handler, uint32_t newMask);
    void cleanUpGarbage();

private:
    EpollManager epollManager_;
    std::map<int, IEventHandler *> handlers_;
    std::set<IEventHandler *> pendingRemovals_;
    std::set<IEventHandler *> activeHandlers_;

private:
    EventDispatcher();
    EventDispatcher(const EventDispatcher &);
    EventDispatcher &operator=(const EventDispatcher &);
};

} // namespace network
