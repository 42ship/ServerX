#pragma once

#include "network/ClientHandler.hpp"
#include "network/IEventHandler.hpp"

namespace http {
class IResponseBody;
}

namespace network {

class CGIHandler : public IEventHandler {
public:
    CGIHandler(http::IResponseBody &body, ClientHandler &client, bool isNPH = true);

    void handleEvent(uint32_t events);
    int getFd() const;

private:
    http::IResponseBody &body_;
    ClientHandler &client_;
    bool isNPH_;
};

} // namespace network
