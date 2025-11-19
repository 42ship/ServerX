#pragma once

#include "network/ClientHandler.hpp"
#include "network/IEventHandler.hpp"

namespace http {
class IResponseBody;
}

namespace network {

class CGIHandler : public IEventHandler {
public:
    CGIHandler(http::IResponseBody &body, ClientHandler &client, bool isNoParseHeaders);

public:
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
    bool isNPH_;
    State state_;
};

} // namespace network
