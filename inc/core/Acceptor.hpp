#pragma once

#include "AEventHandler.hpp"
#include "Socket.hpp"

namespace core {

/**
 * @brief Acceptor inherent from AEventHandler  as a concrete class that accepts new client
 * connections.
 *
 * Listens on a designated port and creates new client connections when connection
 * requests arrive. Acts as a factory for client event handlers for the Reactor class - when a new
 * connection is established, it typically creates and registers a corresponding handler to manage
 * that client's subsequent communication. This implements the connection establishment phase of the
 * Reactor pattern.
 */
class Acceptor : public AEventHandler {
public:
    explicit Acceptor(int port);
    ~Acceptor();

    virtual void handleEvent(uint32_t events);
    virtual int getHandle() const;

private:
    Socket *socket_;
    int port_;

    void setupListeningSocket();
    void acceptNewConnection();

    Acceptor(const Acceptor &);
    Acceptor &operator=(const Acceptor &);
};

} // namespace core
