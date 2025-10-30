#pragma once

#include "AEventHandler.hpp"
#include "http/HttpResponse.hpp"
#include "http/RequestParser.hpp"
#include "http/Router.hpp"
#include <sys/types.h>
#include <vector>

namespace network {

enum ResponseState { NOT_READY, SENDING, SENT };

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
    http::HttpResponse response_; //!< The HTTP response being prepared/sent.

    http::RequestParser reqParser_;
#if 0
    // --- Request State ---
    RequestState requestState_; //!< The current state of request parsing.
    std::string requestBuffer_; //!< Buffer for incoming request data.
    size_t bodyStart_;          //!< Start position of the body in requestBuffer_.
    size_t contentLength_;      //!< Expected length of the request body.
#endif

    // --- Response State ---
    std::vector<char> responseBuffer_; //!< Buffer for the outgoing response.
    ResponseState responseState_;      //!< The current state of response sending.
    size_t sentResponseBytes_;         //!< Number of bytes sent from responseBuffer_.

    // --- Constants ---
    static const size_t IO_BUFFER_SIZE = 8192; //!< Size for read/write chunks.

    // --- Private Methods ---
    /// @brief Handles incoming data on the socket.
    void handleRead();
    /// @brief Handles outgoing data on the socket.
    void handleWrite();
#if 0
    /// @brief Attempts to parse headers from the request buffer.
    void tryParseHeaders();
#endif
    /// @brief Processes a fully parsed request to generate a response.
    void generateResponse();
    /// @brief Sends the contents of the response buffer. @return False on fatal error.
    bool sendResponseBuffer();
    /// @brief Clears the response buffer and resets sent byte count.
    void clearResponseBuffer();
    /// @brief Resets the reactor state for a new request (keep-alive).
    void resetForNewRequest();
    /// @brief Closes the connection and removes it from the dispatcher.
    void closeConnection();
};

} // namespace network
