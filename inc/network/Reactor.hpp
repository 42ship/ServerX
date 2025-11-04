#pragma once

#include "AEventHandler.hpp"
#include "http/RequestParser.hpp"
#include "http/Response.hpp"
#include "http/Router.hpp"
#include <sys/types.h>
#include <vector>

namespace network {

/**
 * @class Reactor
 * @brief Manages the full lifecycle of a single client connection.
 * @details This class acts as a state machine for a client, handling I/O events
 * via epoll. It reads and parses HTTP requests, dispatches them to a router,
 * and manages sending the response back to the client, including streaming
 * large files. Each instance corresponds to one connected client socket.
 */
class Reactor : public AEventHandler {
public:
    Reactor(int clientFd, int port, http::Router const &);
    ~Reactor();

    virtual void handleEvent(uint32_t events);
    virtual int getHandle() const;

private:
    Reactor(const Reactor &);
    Reactor &operator=(const Reactor &);

    // --- Core Connection State ---
    int clientFd_;
    int port_;

    http::Router const &router_;
    http::Request request_;
    http::Response response_; //!< The HTTP response being prepared/sent.

    http::RequestParser reqParser_;

    // --- Response State ---
    struct SendBuffer {
        SendBuffer(size_t initialCapacity = IO_BUFFER_SIZE);

        std::vector<char> buffer;
        size_t sent;

        void reset();
        bool isFullySent();
        enum SendStatus { SEND_DONE, SEND_AGAIN, SEND_ERROR };
        SendStatus send(int clientFd);
    };
    SendBuffer rspBuffer_;

    // --- Constants ---
    static const size_t IO_BUFFER_SIZE = 8192; //!< Size for read/write chunks.

    // --- Private Methods ---
    /// @brief Handles incoming data on the socket.
    void handleRead();
    /// @brief Handles outgoing data on the socket.
    void handleWrite();
    /// @brief Processes a fully parsed request to generate a response.
    void generateResponse();
    /// @brief Resets the reactor state for a new request (keep-alive).
    void resetForNewRequest();
    /// @brief Closes the connection and removes it from the dispatcher.
    void closeConnection();

    void finalizeConnection();

    void handleRequestParsingState(http::RequestParser::RequestState state);
};

} // namespace network
