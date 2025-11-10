#include "network/CGIHandler.hpp"
#include <sys/epoll.h>

namespace network {

CGIHandler::CGIHandler(int fd, ClientHandler &client) : fd_(fd), client_(client) {}

int CGIHandler::getFd() const { return fd_; }

void CGIHandler::handleEvent(uint32_t events) {
    (void)events;
    client_.handleEvent(EPOLLOUT);
}

} // namespace network
