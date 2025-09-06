#pragma once

#include "AEventHandler.hpp"
#include "http/Router.hpp"
#include "http/HttpResponse.hpp"
#include <cstddef>

namespace network {

enum ReadState { READING_HEADERS, READING_BODY, REQUEST_READY };

/**
 * @brief Concrete event handler that manages I/O operations for a connected client.
 *
 * Handles read and write events on an established client connection. Maintains
 * per-client buffers for incoming and outgoing data, processing EPOLLIN events
 * for reading client data and EPOLLOUT events for writing responses. Each instance
 * manages the complete I/O lifecycle for one client connection.
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

    int clientFd_;
    int port_;
    ReadState state_;
    http::Router const &router_;
    http::HttpResponse response_;

    std::string buffer_;
    size_t bodyStart_;
    size_t contentLength_;
    size_t bodyBytesRead_;

    void handleRead();
    void handleWrite();
    void tryParseHeaders();
    void processRequest();
    void cleanup();
    bool sendAll(char const *, size_t len);
};

} // namespace network
