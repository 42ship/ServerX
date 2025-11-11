#include "network/ClientHandler.hpp"
#include "http/ResponseBody.hpp"
#include "http/Router.hpp"
#include "network/CGIHandler.hpp"
#include "network/EventDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

ClientHandler::ClientHandler(int clientFd, int port, std::string const &clientAddr,
                             http::Router const &router)
    : clientFd_(clientFd),
      port_(port),
      clientAddr_(clientAddr),
      router_(router),
      reqParser_(request_, IO_BUFFER_SIZE),
      rspEventSource_(NULL) {
    resetForNewRequest();
    LOG_TRACE("ClientHandler::ClientHandler(" << clientFd_ << "," << port_ << ") from "
                                              << clientAddr_ << ": new connection accepted");
}

ClientHandler::~ClientHandler() {
    LOG_TRACE("ClientHandler::~ClientHandler(" << clientFd_ << "): closed");
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void ClientHandler::closeConnection() {
    LOG_TRACE("ClientHandler::closeConnection(" << clientFd_ << "): removing handler");
    if (rspEventSource_) {
        EventDispatcher::getInstance().removeHandler(rspEventSource_);
        rspEventSource_ = NULL;
    }
    EventDispatcher::getInstance().removeHandler(this);
}

void ClientHandler::handleEvent(uint32_t events) {
    if (events & EPOLLIN) {
        handleRead();
    }
    if (events & EPOLLOUT) {
        handleWrite();
    }
}

int ClientHandler::getFd() const { return clientFd_; }

void ClientHandler::handleRequestParsingState(http::RequestParser::State state) {
    if (state == http::RequestParser::ERROR) {
        LOG_DEBUG("ClientHandler::handleRead(" << clientFd_
                                               << "): state=ERROR, generating error response");
        return generateResponse();
    }
    if (state == http::RequestParser::HEADERS_READY) {
        LOG_DEBUG("ClientHandler::handleRead(" << clientFd_
                                               << "): state=HEADERS_READY, matching location");
        router_.matchServerAndLocation(port_, request_);
        handleRequestParsingState(reqParser_.proceedReadingBody());
    }
    if (state == http::RequestParser::REQUEST_READY) {
        router_.matchServerAndLocation(port_, request_);
        generateResponse();
    }
}

void ClientHandler::handleRead() {
    char read_buffer[IO_BUFFER_SIZE];

    int count = recv(clientFd_, read_buffer, IO_BUFFER_SIZE, 0);
    LOG_TRACE("ClientHandler::handleRead(" << clientFd_ << "): recv " << count << " bytes");
    if (count <= 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (count == 0) {
            LOG_DEBUG("ClientHandler::handleRead(" << clientFd_ << "): client disconnected");
        } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("ClientHandler::handleRead(" << clientFd_
                                                   << "): Recv error: " << strerror(errno));
        }
        closeConnection();
        return;
    }
    handleRequestParsingState(reqParser_.feed(read_buffer, count));
}

void ClientHandler::generateResponse() {
    LOG_TRACE("ClientHandler::generateResponse(" << clientFd_ << "): dispatching to router");
    try {
        // Set remote address in request before dispatch
        request_.remoteAddr(clientAddr_);
        router_.dispatch(port_, request_, response_);
    } catch (std::exception const &e) {
        LOG_ERROR("ClientHandler::generateResponse("
                  << clientFd_ << "): exception during dispatch: " << e.what());
        if (response_.status() < 400) {
            response_.status(http::INTERNAL_SERVER_ERROR, e.what());
        }
    } catch (...) {
        LOG_ERROR("ClientHandler::generateResponse(" << clientFd_
                                                     << "): unknown exception during dispatch");
        if (response_.status() < 400) {
            response_.status(http::INTERNAL_SERVER_ERROR);
        }
    }

    LOG_TRACE("ClientHandler::generateResponse(" << clientFd_ << "): building headers");
    response_.buildHeaders(rspBuffer_.buffer);
    LOG_DEBUG("ClientHandler::generateResponse(" << clientFd_ << "): modifying fd to EPOLLOUT");
    EventDispatcher::getInstance().setSendingData(this);
    http::IResponseBody *body = response_.body();
    if (body && !body->isDone() && body->getEventSourceFd() != -1) {
        rspEventSource_ = new CGIHandler(body->getEventSourceFd(), *this);
        EventDispatcher::getInstance().registerHandler(rspEventSource_);
        EventDispatcher::getInstance().modifyHandler(this, 0);
    }
}

void ClientHandler::handleWrite() {
    LOG_TRACE("ClientHandler::handleWrite(" << clientFd_ << "): trying to send from buffer...");
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
        LOG_TRACE("ClientHandler::handleWrite(" << clientFd_
                                                << "): no body or body is done, finalizing");
        return finalizeConnection();
    }
    rspBuffer_.buffer.resize(rspBuffer_.buffer.capacity());
    ssize_t read = body->read(rspBuffer_.buffer.data(), rspBuffer_.buffer.capacity());
    if (read <= 0) {
        return finalizeConnection();
    }
    LOG_TRACE("ClientHandler::handleWrite(" << clientFd_ << "): read " << read
                                            << " bytes from body, attempting to send");
    rspBuffer_.buffer.resize(read);
    status = rspBuffer_.send(clientFd_);
    if (status == SendBuffer::SEND_ERROR) {
        return closeConnection();
    }
}

void ClientHandler::finalizeConnection() {
    LOG_TRACE("ClientHandler::finalizeConnection(" << clientFd_ << "): finalizing");
    if (request_.headers().get("Connection") == "keep-alive") {
        LOG_DEBUG("ClientHandler::finalizeConnection(" << clientFd_ << "): keep-alive, resetting");
        EventDispatcher::getInstance().setSendingData(this);
        resetForNewRequest();
    } else {
        LOG_DEBUG("ClientHandler::finalizeConnection(" << clientFd_ << "): closing connection");
        closeConnection();
    }
}

ClientHandler::SendBuffer::SendBuffer(size_t initialCapacity) : sent(0) {
    buffer.reserve(initialCapacity);
}

void ClientHandler::SendBuffer::reset() {
    buffer.clear();
    sent = 0;
}

bool ClientHandler::SendBuffer::isFullySent() { return sent == buffer.size(); }

ClientHandler::SendBuffer::SendStatus ClientHandler::SendBuffer::send(int clientFd) {
    if (isFullySent())
        return SEND_DONE;
    size_t bytes_to_send = buffer.size() - sent;
    ssize_t bytes_sent = ::send(clientFd, buffer.data() + sent, bytes_to_send, 0);
    if (bytes_sent < 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_DEBUG("ClientHandler::SendBuffer::send(" << clientFd
                                                         << "): client's buffer is full");
            return SEND_AGAIN;
        }
        LOG_ERROR("ClientHandler::SendBuffer::send(" << clientFd << "): " << strerror(errno));
        return SEND_ERROR;
    }
    LOG_TRACE("ClientHandler::SendBuffer::send(" << clientFd << ") sent " << bytes_sent
                                                 << " bytes");
    sent += bytes_to_send;
    return (isFullySent() ? SEND_DONE : SEND_AGAIN);
}

void ClientHandler::resetForNewRequest() {
    LOG_TRACE("ClientHandler::resetForNewRequest(" << clientFd_ << "): resetting for keep-alive");
    EventDispatcher::getInstance().removeHandler(rspEventSource_);
    rspEventSource_ = NULL;
    rspBuffer_.reset();
    reqParser_.reset();
    response_.clear();
    request_.clear();
}

} // namespace network
