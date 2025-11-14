#pragma once

#include "IEventHandler.hpp"
#include "http/RequestParser.hpp"
#include "http/Response.hpp"
#include "http/Router.hpp"

namespace network {

/**
 * @class ClientHandler
 * @brief Manages the full lifecycle of a single client connection.
 * @details This class acts as a state machine for a client, handling I/O events
 * via epoll. It reads and parses HTTP requests, dispatches them to a router,
 * and manages sending the response back to the client, including streaming
 * large files. Each instance corresponds to one connected client socket.
 */
class ClientHandler : public IEventHandler {
public:
    ClientHandler(int clientFd, int port, http::Router const &);
    ~ClientHandler();

    virtual void handleEvent(uint32_t events);
    virtual int getFd() const;

    /**
     * @brief Pushes data into the client's send buffer.
     * This is called by the CGIHandler when it reads from the pipe.
     */
    void pushToSendBuffer(const char *data, size_t length);

    /**
     * @brief Checks if the send buffer is over its high-water mark.
     * This is the backpressure mechanism.
     */
    bool isSendBufferFull() const;

    /**
     * @brief Called by CGIHandler when the CGI script finishes (EOF).
     */
    void onCgiComplete();

private:
    ClientHandler(const ClientHandler &);
    ClientHandler &operator=(const ClientHandler &);

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

    IEventHandler *rspEventSource_;

    // --- Private Methods ---
    /// @brief Handles incoming data on the socket.
    void handleRead();
    /// @brief Handles outgoing data on the socket.
    void handleWritePassive();

    void handleWriteCGI();

    /// @brief Processes a fully parsed request to generate a response.
    void generateResponse();
    /// @brief Sends the contents of the response buffer. @return False on fatal error.
    bool sendResponseBuffer();
    /// @brief Clears the response buffer and resets sent byte count.
    void clearResponseBuffer();
    /// @brief Resets the ClientHandler state for a new request (keep-alive).
    void resetForNewRequest();
    /// @brief Closes the connection and removes it from the dispatcher.
    void closeConnection();

    void finalizeConnection();

    void handleRequestParsingState(http::RequestParser::State state);
};

} // namespace network
