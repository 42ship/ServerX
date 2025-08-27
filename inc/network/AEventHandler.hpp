#pragma once

#include <stdint.h>

namespace network {

/**
 * @brief Abstract base class defining the interface for event-driven service handlers.
 *
 * Provides the hook method interface that concrete handlers (Acceptor, Reactor) must implement to
 * process specific types of events. Event handlers register with the InitiationDispatcher and
 * receive callbacks when events occur, detected by the epoll() mechanism, on their associated file
 * descriptors (listeningFd, clientFd). This forms the core abstraction in the Reactor pattern for
 * service-specific processing.
 *
 *  virtual void handleEvent(uint32_t events) = 0;  // Pure virtual - must implement
 *  virtual int getHandle() const = 0;               // Returns file
 */
class AEventHandler {
public:
    virtual ~AEventHandler();
    virtual void handleEvent(uint32_t events) = 0;
    virtual int getHandle() const = 0;

protected:
    AEventHandler();

private:
    AEventHandler(const AEventHandler &);
    AEventHandler &operator=(const AEventHandler &);
};

} // namespace network
