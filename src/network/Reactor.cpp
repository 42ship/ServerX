#include "network/Reactor.hpp"
#include "http/ResponseBody.hpp"
#include "http/Router.hpp"
#include "network/InitiationDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

Reactor::Reactor(int clientFd, int port, http::Router const &router)
    : clientFd_(clientFd), port_(port), router_(router), reqParser_(request_, IO_BUFFER_SIZE) {
    resetForNewRequest();
    LOG_TRACE("Reactor::Reactor(" << clientFd_ << "," << port_ << "): new connection accepted")
}

Reactor::~Reactor() {
    LOG_TRACE("Reactor::~Reactor(" << clientFd_ << "," << port_ << "): closed")
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void Reactor::closeConnection() { InitiationDispatcher::getInstance().removeHandler(clientFd_); }

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
    char read_buffer[IO_BUFFER_SIZE];

    int count = recv(clientFd_, read_buffer, IO_BUFFER_SIZE, 0);
    LOG_TRACE("Reactor::handleRead: read " << count << " bytes")
    if (count <= 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (count == 0) {
            LOG_TRACE("Reactor::handleRead: client disconnected on fd: " << clientFd_);
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("Recv error on fd " << clientFd_ << ": " << strerror(errno));
        }
        closeConnection();
        return;
    }
    LOG_DEBUG("Received " << count << " bytes from fd: " << clientFd_);
    http::RequestParser::RequestState state = reqParser_.addIncomingChunk(read_buffer, count);
    if (state == http::RequestParser::ERROR) {
        return closeConnection();
    } else if (state == http::RequestParser::HEADERS_READY) {
        // TODO: validate things
        // This matchServerAndLocation will be useful for the reqParser because if there is a body
        // it would use it to identify the specific body limit but it's not yet implemented
        router_.matchServerAndLocation(port_, request_);
        reqParser_.proceedReadingBody();
    } else if (state == http::RequestParser::HEADERS_READY) {
        generateResponse();
    }
}

void Reactor::generateResponse() {
    try {
        router_.dispatch(port_, request_, response_);
    } catch (...) {
        closeConnection();
    }
    response_.buildHeaders(rspBuffer_.buffer);
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
    http::IResponseBody *body = response_.body();
    if (!body || body->isDone()) {
        return finalizeConnection();
    }
    rspBuffer_.buffer.resize(rspBuffer_.buffer.capacity());
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
    if (response_.headers().get("Connection") == "keep-alive") {
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
    LOG_TRACE("Reactor::SendBuffer::send(" << clientFd << ") sent " << bytes_sent << " bytes")
    if (bytes_sent < 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_DEBUG("Reactor::SendBuffer::send(" << clientFd << "): client's buffer is full")
            return SEND_AGAIN;
        }
        LOG_ERROR("Reactor::SendBuffer::send(" << clientFd << "): " << strerror(errno))
        return SEND_ERROR;
    }
    sent += bytes_to_send;
    return (isFullySent() ? SEND_DONE : SEND_AGAIN);
}

void Reactor::resetForNewRequest() {
    rspBuffer_.reset();
    reqParser_.reset();
    response_.clear();
    request_.clear();
}

} // namespace network
