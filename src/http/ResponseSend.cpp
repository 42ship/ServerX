#include "http/ResponseSend.hpp"

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <iostream>

bool ResponseSend::sendAll(int clientFd, const std::string &data) {
    size_t sent = 0;
    size_t responseSize = data.size();
    const char *buf = data.c_str();
    while (sent < responseSize) {
        ssize_t n = send(clientFd, buf + sent, responseSize - sent,
#ifdef MSG_NOSIGNAL
                         MSG_NOSIGNAL
#else
                         0
#endif
        );
        if (n <= 0)
            return false;
        sent += static_cast<size_t>(n);
    }
    return true;
}

long ResponseSend::sendSome(int clientFd, const std::string &data) {
    errno = 0; // Reset errno before send
    ssize_t sent = 0;
    sent = send(clientFd, data.c_str(), data.size(),
#ifdef MSG_NOSIGNAL
                MSG_NOSIGNAL
#else
                0
#endif
    );
    if (sent < 0) { // Handle specific errors
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return 0; // Non-blocking send, try again later
        return -1;    // Other errors, return -1 to indicate failure
    }
    if (sent == 0)
        return -1; // No data sent, return -1 to indicate failure
    return static_cast<long>(sent);
}
