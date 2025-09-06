#include "network/Reactor.hpp"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/Router.hpp"
#include "http/RouterResult.hpp"
#include "network/InitiationDispatcher.hpp"

namespace network {

Reactor::Reactor(int clientFd, int port, http::Router const &router)
    : clientFd_(clientFd), port_(port), state_(READING_HEADERS), router_(router) {
}

Reactor::~Reactor() {
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void Reactor::cleanup() {
    InitiationDispatcher::getInstance().removeHandler(clientFd_);
}

void Reactor::handleEvent(uint32_t events) {
    if (events & EPOLLIN) {
        handleRead();
    }
    if (events & EPOLLOUT) {
        handleWrite();
    }
}

int Reactor::getHandle() const {
    return clientFd_;
}

// TODO review recv_cb function logic
void Reactor::handleRead() {
    char read_buffer[8192];

    int count = recv(clientFd_, read_buffer, 8192, 0);
    if (count <= 0) {
        if (count < 0 && (errno != EAGAIN && errno != EWOULDBLOCK)) {
            std::cerr << "Recv error: " << strerror(errno) << std::endl;
        }
        cleanup();
        return;
    }
    buffer_.insert(buffer_.end(), read_buffer, read_buffer + count);
    if (state_ == READING_HEADERS) {
        tryParseHeaders();
    }
    if (state_ == READING_BODY) {
        if (buffer_.size() - bodyStart_ >= contentLength_)
            state_ = REQUEST_READY;
    }
    if (state_ == REQUEST_READY) {
        processRequest();
    }
}

void Reactor::tryParseHeaders() {
    bodyStart_ = buffer_.find("\r\n\r\n", 0, 4);

    if (bodyStart_ != std::string::npos) {
        bodyStart_ += 4;
        state_ = READING_BODY;

        static const std::string cl_key = "Content-Length: ";
        size_t pos = buffer_.find(cl_key);
        if (pos != std::string::npos) {
            contentLength_ = std::strtol(buffer_.c_str() + pos + cl_key.length(), NULL, 10);
        } else
            contentLength_ = 0;
    }
}

void Reactor::processRequest() {
    http::HttpRequest request = http::HttpRequest::parse(buffer_);
    http::RouterResult result = router_.route(port_, request);
    response_ = result.handler.handle(request, result);
    InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLOUT);
}

// TODO review send_cb function logic
void Reactor::handleWrite() {
    std::string const &headers = response_.buildHeaders();
    sendAll(headers.c_str(), headers.size());
    switch (response_.getBodyType()) {
    case http::BODY_NONE: {
        break;
    }
    case http::BODY_IN_MEMORY: {
        sendAll(&(*response_.inMemoryBody.data)[0], response_.inMemoryBody.data->size());
        break;
    }
    case http::BODY_FROM_FILE: {
        char buffer[8192];
        ssize_t bytes_read;
        while ((bytes_read = read(response_.fileBody.fd, buffer, sizeof(buffer))) > 0) {
            if (!sendAll(buffer, bytes_read))
                break;
        }
        close(response_.fileBody.fd);
        break;
    }
    case http::BODY_FROM_CGI: {
        break;
    }
    }
    if (response_.getHeaders()["Connection"] == "keep-alive") {
        InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLIN);
        state_ = READING_HEADERS;
    } else
        cleanup();
}

bool Reactor::sendAll(char const *s, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t just_sent = send(clientFd_, s + sent, len - sent, 0);
        if (just_sent < 0)
            return false;
        sent += just_sent;
    }
    return true;
}

} // namespace network
