#pragma once

#include "IEventHandler.hpp"
#include "config/ServerBlock.hpp"
#include "http/Router.hpp"
#include "network/Socket.hpp"

namespace network {

/**
 * @brief Acceptor inherent from IEventHandler  as a concrete class that accepts new client
 * connections.
 *
 * Listens on a designated port and creates new client connections when connection
 * requests arrive. Acts as a factory for client event handlers for the ClientHandler class - when a new
 * connection is established, it typically creates and registers a corresponding handler to manage
 * that client's subsequent communication. This implements the connection establishment phase of the
 * ClientHandler pattern.
 */
class Acceptor : public IEventHandler {
public:
    Acceptor(int port, http::Router const &);
    Acceptor(config::ServerBlock const &, http::Router const &);
    ~Acceptor();

    void handleEvent(uint32_t events);
    int getFd() const;

private:
    Socket socket_;
    int port_;
    http::Router const &router_;

    void acceptNewConnection();

    Acceptor(const Acceptor &);
    Acceptor &operator=(const Acceptor &);
};

} // namespace network
