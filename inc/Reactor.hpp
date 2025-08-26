#pragma once

#include "AEventHandler.hpp"

#define BUFFER_LENGTH 1024

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
    explicit Reactor(int clientFd);
    ~Reactor();

    virtual void handleEvent(uint32_t events);
    virtual int getHandle() const;

private : int clientFd_;
    char readBuffer_[BUFFER_LENGTH];
    int readLength_;
    char writeBuffer_[BUFFER_LENGTH];
    int writeLength_;

    void handleRead();
    void handleWrite();

    Reactor(const Reactor &);
    Reactor &operator=(const Reactor &);
};