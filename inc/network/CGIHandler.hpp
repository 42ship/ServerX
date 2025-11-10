#pragma once

#include "network/ClientHandler.hpp"
#include "network/IEventHandler.hpp"

namespace network {

class CGIHandler : public IEventHandler {
public:
    CGIHandler(int fd, ClientHandler &client);

    void handleEvent(uint32_t events);
    int getFd() const;

private:
    int fd_;
    ClientHandler &client_;
};

} // namespace network
