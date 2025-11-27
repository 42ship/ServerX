#pragma once

#include "network/ClientHandler.hpp"
#include "network/IEventHandler.hpp"

namespace network {

class CGIHandler : public IEventHandler {
public:
    CGIHandler(http::IResponseBody &body, ClientHandler &client, bool hasHeaderParsing);

    void handleEvent(uint32_t events);
    int getFd() const;

private:
    enum State { READING_HEADERS, STREAMING_BODY, COMPLETE };

private:
    void handleRead();

private:
    http::IResponseBody &body_;
    ClientHandler &client_;
    std::string headerBuffer_;
    State state_;
    int fd_;
};

} // namespace network
