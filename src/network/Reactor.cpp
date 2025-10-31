#include "network/Reactor.hpp"
#include "http/HttpResponse.hpp"
#include "http/ResponseBody.hpp"
#include "http/Router.hpp"
#include "network/InitiationDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

Reactor::Reactor(int clientFd, int port, http::Router const &router)
    : clientFd_(clientFd), port_(port), router_(router), reqParser_(8192) {
    resetForNewRequest();
    LOG_TRACE("New connection accepted on fd: " << clientFd_);
}

Reactor::~Reactor() {
    LOG_TRACE("Closing connection on fd: " << clientFd_);
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void Reactor::closeConnection() {
    LOG_TRACE("Cleaning up handler for fd: " << clientFd_);
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

int Reactor::getHandle() const { return clientFd_; }

void Reactor::handleRead() {
    LOG_TRACE("Handling read event on fd: " << clientFd_);
    char read_buffer[IO_BUFFER_SIZE];

    int count = recv(clientFd_, read_buffer, IO_BUFFER_SIZE, 0);
    if (count <= 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (count == 0) {
            LOG_INFO("Client disconnected on fd: " << clientFd_);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("Recv error on fd " << clientFd_ << ": " << strerror(errno));
        }
        closeConnection();
        return;
    }
    LOG_DEBUG("Received " << count << " bytes from fd: " << clientFd_);
    http::RequestParser::RequestState state = reqParser_.addIncomingChunk(read_buffer, count);
    if (state == http::RequestParser::REQUEST_READY || state == http::RequestParser::ERROR) {
        generateResponse();
    } else if (state == http::RequestParser::HEADERS_READY) {
        // TODO: validate things
        reqParser_.proceedReadingBody();
    }
}

void Reactor::generateResponse() {
    router_.route(port_, reqParser_.getRequestContext());
#if 0
    std::string const &headers = response_.buildHeaders();
    responseBuffer_.assign(headers.begin(), headers.end());

    if (response_.getBodyType() == http::BODY_IN_MEMORY && response_.inMemoryBody.data) {
        responseBuffer_.insert(responseBuffer_.end(), response_.inMemoryBody.data->begin(),
                               response_.inMemoryBody.data->end());
    }
    LOG_DEBUG("Generated response for fd: " << clientFd_ << " status: " << response_.getStatus());
#endif
    InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLOUT);
}

void Reactor::handleWrite() {
    SendBuffer::SendStatus status = rspBuffer_.send(clientFd_);

    if (status == SendBuffer::SEND_ERROR) {
        return closeConnection();
    }
    if (status == SendBuffer::SEND_AGAIN) {
        return;
    }
    if (status != SendBuffer::SEND_DONE) {
        return;
    }
    rspBuffer_.reset();
    http::IResponseBody *body = response_.body;
    if (!body || body->isDone()) {
        return finalizeConnection();
    }
    ssize_t read = body->read(rspBuffer_.buffer.data(), rspBuffer_.buffer.capacity());
    if (read <= 0) {
        return finalizeConnection();
    }
    rspBuffer_.buffer.resize(read);
    status = rspBuffer_.send(clientFd_);
    if (status == SendBuffer::SEND_ERROR) {
        return closeConnection();
    }
}

void Reactor::finalizeConnection() {
    if (response_.headers.get("Connection") == "keep-alive") {
        InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLIN);
        resetForNewRequest();
    } else
        closeConnection();
}

Reactor::SendBuffer::SendBuffer(size_t initialCapacity) : sent(0) {
    buffer.reserve(initialCapacity);
}

void Reactor::SendBuffer::reset() {
    buffer.clear();
    sent = 0;
}

bool Reactor::SendBuffer::isFullySent() { return sent == buffer.size(); }

Reactor::SendBuffer::SendStatus Reactor::SendBuffer::send(int clientFd) {
    if (isFullySent())
        return SEND_DONE;
    size_t bytes_to_send = buffer.size() - sent;
    ssize_t bytes_sent = ::send(clientFd, buffer.data() + sent, bytes_to_send, 0);
    if (bytes_sent < 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_TRACE("Send buffer is full for fd " << clientFd << ", will try again later.");
            return SEND_AGAIN;
        }
        LOG_ERROR("Send error on fd " << clientFd << ": " << strerror(errno));
        return SEND_ERROR;
    }
    bytes_sent += bytes_to_send;
    return (isFullySent() ? SEND_DONE : SEND_AGAIN);
}

void Reactor::resetForNewRequest() {
    rspBuffer_.reset();
    reqParser_.reset();
#if 0
    // TODO:
    response_.clear();
    request_.clear();
#endif
}

} // namespace network
