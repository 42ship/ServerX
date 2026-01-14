#pragma once

#include "IEventHandler.hpp"
#include "http/Headers.hpp"
#include "http/HttpStatus.hpp"
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
    // =========================================================================
    // Lifecycle & Interface
    // =========================================================================
public:
    ClientHandler(int clientFd, int port, std::string const &clientAddr, http::Router const &);
    virtual ~ClientHandler();

    // IEventHandler implementation
    virtual void handleEvent(uint32_t events);
    virtual int getFd() const;

    // =========================================================================
    // CGI / Buffer Interaction (Called by external helpers)
    // =========================================================================
public:
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

    /**
     * @brief Notification that CGI headers have been fully parsed.
     */
    void onCgiHeadersParsed(http::Headers const &);

    /**
     * @brief Triggers an error response (e.g., 404, 500).
     */
    void handleError(http::HttpStatus);

    // =========================================================================
    // Internal Helper Structures
    // =========================================================================
private:
    // Helper to manage the output buffer logic
    struct SendBuffer {
        std::vector<char> buffer;
        size_t sent;

        explicit SendBuffer(size_t initialCapacity);
        void reset();
        bool isFullySent() const;

        enum SendStatus { SEND_DONE, SEND_AGAIN, SEND_ERROR };
        SendStatus send(int clientFd);
    };

    // Helper to group CGI/Event Source state
    struct CgiState {
        IEventHandler *handler; // The worker handling the CGI script
        bool isDone;

        CgiState();
        void clear();
        void remove();
    };

private:
    // Core Socket State
    int clientFd_;
    int port_;
    std::string clientAddr_;
    bool headersSent_;

    // HTTP State
    http::Router const &router_;
    http::Request request_;
    http::Response response_;
    http::RequestParser reqParser_;

    // Response State
    bool isKeepAlive_;
    SendBuffer rspBuffer_;
    CgiState cgiState_;

    static const size_t IO_BUFFER_SIZE = 8192;

private:
    /// @brief Handles incoming data on the socket.
    void handleRead();
    /// @brief Handles outgoing data on the socket.
    void handleStaticResponseWrite();
    void handleCgiResponseWrite();

    // Logic Steps
    /// @brief Processes a fully parsed request to generate a response.
    void generateResponse();
    void handleRequestParsingState(http::RequestParser::State state);
    bool setupCgiHandler(http::IResponseBody *body);
    void setupStaticResponse();

    // Connection Lifecycle
    /// @brief Resets the ClientHandler state for a new request (keep-alive).
    void resetForNewRequest();
    /// @brief Closes the connection and removes it from the dispatcher.
    void closeConnection();
    void finalizeConnection();

    ClientHandler(const ClientHandler &);
    ClientHandler &operator=(const ClientHandler &);

    std::string getLogSignature() const;
};

} // namespace network
