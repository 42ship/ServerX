#include "network/ClientHandler.hpp"

#include "http/HttpRequest.hpp"
#include "http/HttpResponse.hpp"
#include "http/Router.hpp"
#include "http/RouterResult.hpp"
#include "http/error_pages.hpp"
#include "network/EventDispatcher.hpp"
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

ClientHandler::ClientHandler(int clientFd, int port, http::Router const &router)
    : clientFd_(clientFd), port_(port), router_(router), responseBuffer_(8192) {
    resetForNewRequest();
    LOG_TRACE("New connection accepted on fd: " << clientFd_);
}

ClientHandler::~ClientHandler() {
    LOG_TRACE("Closing connection on fd: " << clientFd_);
    if (clientFd_ >= 0) {
        close(clientFd_);
    }
}

void ClientHandler::closeConnection() {
    LOG_TRACE("Cleaning up handler for fd: " << clientFd_);
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

void ClientHandler::handleRead() {
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
    requestBuffer_.insert(requestBuffer_.end(), read_buffer, read_buffer + count);
    if (requestState_ == READING_HEADERS) {
        tryParseHeaders();
    }
    if (requestState_ == READING_BODY) {
        if (requestBuffer_.size() - bodyStart_ >= contentLength_)
            requestState_ = REQUEST_READY;
    }
    if (requestState_ == REQUEST_READY) {
        LOG_TRACE("Request fully received on fd: " << clientFd_ << ", processing...");
        generateResponse();
    }
}

void ClientHandler::tryParseHeaders() {
    bodyStart_ = requestBuffer_.find("\r\n\r\n", 0, 4);

    if (bodyStart_ != std::string::npos) {
        LOG_DEBUG("Headers parsed for fd: " << clientFd_);
        bodyStart_ += 4;
        requestState_ = READING_BODY;

        static const std::string cl_key = "Content-Length: ";
        size_t pos = requestBuffer_.find(cl_key);
        if (pos != std::string::npos) {
            contentLength_ = std::strtol(requestBuffer_.c_str() + pos + cl_key.length(), NULL, 10);
        } else
            contentLength_ = 0;
        LOG_DEBUG("Request body size: " << contentLength_ << " bytes on fd: " << clientFd_);
    }
}

void ClientHandler::generateResponse() {
    try {
        LOG_DEBUG(requestBuffer_);
        http::HttpRequest request = http::HttpRequest::parse(requestBuffer_);
        http::RouterResult result = router_.route(port_, request);
        response_ = result.handler.handle(request, result);
    } catch (std::exception const &e) {
        LOG_ERROR("ClientHandler::generateResponse(): " << e.what());
        response_ =
            http::error_pages::generateErrorResponse(http::INTERNAL_SERVER_ERROR, "HTTP/1.1");
    } catch (...) {
        LOG_ERROR("ClientHandler::generateResponse(): \"Internal server error\"");
        response_ =
            http::error_pages::generateErrorResponse(http::INTERNAL_SERVER_ERROR, "HTTP/1.1");
    }

    std::string const &headers = response_.buildHeaders();
    responseBuffer_.assign(headers.begin(), headers.end());

    if (response_.getBodyType() == http::BODY_IN_MEMORY && response_.inMemoryBody.data) {
        responseBuffer_.insert(responseBuffer_.end(), response_.inMemoryBody.data->begin(),
                               response_.inMemoryBody.data->end());
    }
    LOG_DEBUG("Generated response for fd: " << clientFd_ << " status: " << response_.getStatus());
    responseState_ = SENDING;
    sentResponseBytes_ = 0;
    EventDispatcher::getInstance().setSendingData(this);
}

void ClientHandler::handleWrite() {
    if (responseState_ == NOT_READY)
        return;
    if (sentResponseBytes_ < responseBuffer_.size()) {
        LOG_TRACE("Sending response chunk to fd: " << clientFd_);
        if (!sendResponseBuffer())
            return;
    }
    if (sentResponseBytes_ == responseBuffer_.size()) {
        clearResponseBuffer();
        if (response_.getBodyType() == http::BODY_FROM_FILE) {
            responseBuffer_.resize(IO_BUFFER_SIZE);
            ssize_t bytes_read =
                read(response_.fileBody.fd, responseBuffer_.data(), responseBuffer_.size());
            if (bytes_read > 0) {
                LOG_DEBUG("Read " << bytes_read << " bytes from file for fd: " << clientFd_);
                responseBuffer_.resize(bytes_read);
                if (!sendResponseBuffer())
                    return;
            } else {
                if (bytes_read < 0) {
                    LOG_ERROR("File read error for fd " << clientFd_ << ": " << strerror(errno));
                } else {
                    LOG_DEBUG("Finished streaming file to fd: " << clientFd_);
                }
                close(response_.fileBody.fd);
                responseState_ = SENT;
            }
        } else
            responseState_ = SENT;
    }
    if (responseState_ != SENT)
        return;
    LOG_DEBUG("Response fully sent to fd: " << clientFd_);
    if (response_.getHeaders()["Connection"] == "keep-alive") {
        LOG_DEBUG("Keep-alive enabled. Resetting state for fd: " << clientFd_);
        EventDispatcher::getInstance().setReceivingData(this);
        resetForNewRequest();
    } else
        closeConnection();
}

bool ClientHandler::sendResponseBuffer() {
    if (sentResponseBytes_ >= responseBuffer_.size()) {
        return true; // Nothing to send
    }
    ssize_t sent = ::send(clientFd_, responseBuffer_.data() + sentResponseBytes_,
                          responseBuffer_.size() - sentResponseBytes_, 0);
    if (sent < 0) {
        // NOTE: to remove for evaluation; errno after recv/send is not allowed
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_TRACE("Send buffer is full for fd " << clientFd_ << ", will try again later.");
        } else {
            LOG_ERROR("Send error on fd " << clientFd_ << ": " << strerror(errno));
            closeConnection();
        }
        return false;
    }
    sentResponseBytes_ += sent;
    LOG_DEBUG("Sent " << sent << " bytes to fd: " << clientFd_);
    return true;
}

void ClientHandler::clearResponseBuffer() {
    responseBuffer_.clear();
    sentResponseBytes_ = 0;
}

void ClientHandler::resetForNewRequest() {
    requestBuffer_.clear();
    responseBuffer_.clear();
    sentResponseBytes_ = 0;
    contentLength_ = 0;
    bodyStart_ = 0;
    requestState_ = READING_HEADERS;
    responseState_ = NOT_READY;
    response_ = http::HttpResponse();
}

} // namespace network
