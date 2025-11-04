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
    LOG_TRACE("Reactor::Reactor(" << clientFd_ << "," << port_ << "): new connection accepted");
}

Reactor::~Reactor() {
    LOG_TRACE("Reactor::~Reactor(" << clientFd_ << "): closed");
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void Reactor::closeConnection() {
    LOG_TRACE("Reactor::closeConnection(" << clientFd_ << "): removing handler");
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
    char read_buffer[IO_BUFFER_SIZE];

    int count = recv(clientFd_, read_buffer, IO_BUFFER_SIZE, 0);
    LOG_TRACE("Reactor::handleRead(" << clientFd_ << "): recv " << count << " bytes");
    if (count <= 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (count == 0) {
            LOG_DEBUG("Reactor::handleRead(" << clientFd_ << "): client disconnected");
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("Reactor::handleRead(" << clientFd_ << "): Recv error: " << strerror(errno));
        }
        closeConnection();
        return;
    }
    http::RequestParser::RequestState state = reqParser_.addIncomingChunk(read_buffer, count);

    if (state == http::RequestParser::ERROR) {
        LOG_DEBUG("Reactor::handleRead(" << clientFd_ << "): state=ERROR, closing connection");
        return closeConnection();
    } else if (state == http::RequestParser::HEADERS_READY) {
        LOG_DEBUG("Reactor::handleRead(" << clientFd_
                                         << "): state=HEADERS_READY, matching location");
        // TODO: validate things
        // This matchServerAndLocation will be useful for the reqParser because if there is a body
        // it would use it to identify the specific body limit but it's not yet implemented
        router_.matchServerAndLocation(port_, request_);
        reqParser_.proceedReadingBody();
    } else if (state == http::RequestParser::REQUEST_READY) {
        generateResponse();
    }
}

void Reactor::generateResponse() {
    LOG_TRACE("Reactor::generateResponse(" << clientFd_ << "): dispatching to router");
    try {
        router_.dispatch(port_, request_, response_);
    } catch (std::exception const &e) {
        LOG_ERROR("Reactor::generateResponse(" << clientFd_
                                               << "): exception during dispatch: " << e.what());
        if (response_.status() < 400) {
            response_.status(http::INTERNAL_SERVER_ERROR, e.what());
        }
    } catch (...) {
        LOG_ERROR("Reactor::generateResponse(" << clientFd_
                                               << "): unknown exception during dispatch");
        if (response_.status() < 400) {
            response_.status(http::INTERNAL_SERVER_ERROR);
        }
    }

    LOG_TRACE("Reactor::generateResponse(" << clientFd_ << "): building headers");
    response_.buildHeaders(rspBuffer_.buffer);
    LOG_DEBUG("Reactor::generateResponse(" << clientFd_ << "): modifying fd to EPOLLOUT");
    InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLOUT);
}

void Reactor::handleWrite() {
    LOG_TRACE("Reactor::handleWrite(" << clientFd_ << "): trying to send from buffer...");
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
        LOG_TRACE("Reactor::handleWrite(" << clientFd_ << "): no body or body is done, finalizing");
        return finalizeConnection();
    }
    rspBuffer_.buffer.resize(rspBuffer_.buffer.capacity());
    ssize_t read = body->read(rspBuffer_.buffer.data(), rspBuffer_.buffer.capacity());
    if (read <= 0) {
        return finalizeConnection();
    }
    LOG_TRACE("Reactor::handleWrite(" << clientFd_ << "): read " << read
                                      << " bytes from body, attempting to send");
    rspBuffer_.buffer.resize(read);
    status = rspBuffer_.send(clientFd_);
    if (status == SendBuffer::SEND_ERROR) {
        return closeConnection();
    }
}

void Reactor::finalizeConnection() {
    LOG_TRACE("Reactor::finalizeConnection(" << clientFd_ << "): finalizing");
    if (response_.headers().get("Connection") == "keep-alive") {
        LOG_DEBUG("Reactor::finalizeConnection(" << clientFd_ << "): keep-alive, resetting");
        InitiationDispatcher::getInstance().getEpollManager().modifyFd(clientFd_, EPOLLIN);
        resetForNewRequest();
    } else {
        LOG_DEBUG("Reactor::finalizeConnection(" << clientFd_ << "): closing connection");
        closeConnection();
    }
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
            LOG_DEBUG("Reactor::SendBuffer::send(" << clientFd << "): client's buffer is full");
            return SEND_AGAIN;
        }
        LOG_ERROR("Reactor::SendBuffer::send(" << clientFd << "): " << strerror(errno))
        return SEND_ERROR;
    }
    LOG_TRACE("Reactor::SendBuffer::send(" << clientFd << ") sent " << bytes_sent << " bytes");
    sent += bytes_to_send;
    return (isFullySent() ? SEND_DONE : SEND_AGAIN);
}

void Reactor::resetForNewRequest() {
    LOG_TRACE("Reactor::resetForNewRequest(" << clientFd_ << "): resetting for keep-alive");
    rspBuffer_.reset();
    reqParser_.reset();
    response_.clear();
    request_.clear();
}

} // namespace network
