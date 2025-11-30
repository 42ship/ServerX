#pragma once

#include <stdint.h>

namespace network {

/**
 * @brief Abstract base class defining the interface for event-driven service handlers.
 *
 * Provides the hook method interface that concrete handlers (Acceptor, ClientHandler) must
 * implement to process specific types of events. Event handlers register with the
 * InitiationDispatcher and receive callbacks when events occur, detected by the epoll() mechanism,
 * on their associated file descriptors (listeningFd, clientFd). This forms the core abstraction in
 * the ClientHandler pattern for service-specific processing.
 *
 *  virtual void handleEvent(uint32_t events) = 0;  // Pure virtual - must implement
 *  virtual int getFd() const = 0;               // Returns file
 */
class IEventHandler {
public:
    virtual ~IEventHandler();
    virtual void handleEvent(uint32_t events) = 0;
    virtual int getFd() const = 0;

protected:
    IEventHandler();

private:
    IEventHandler(const IEventHandler &);
    IEventHandler &operator=(const IEventHandler &);

    friend class EventDispatcher;

    /**
     * @brief Shadow State.
     * Stores the events currently registered in epoll.
     * Prevents redundant syscalls.
     */
    uint32_t registeredEvents_;
};

} // namespace network
