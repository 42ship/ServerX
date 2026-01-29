#include "network/ClientHandler.hpp"
#include "common/string.hpp"
#include "http/HttpStatus.hpp"
#include "http/ResponseBody.hpp"
#include "http/Router.hpp"
#include "network/CGIHandler.hpp"
#include "network/EventDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

namespace network {

// =============================================================================
// Helper Struct Implementation
// =============================================================================

ClientHandler::SendBuffer::SendBuffer(size_t initialCapacity) : sent(0) {
    buffer.reserve(initialCapacity);
}

void ClientHandler::SendBuffer::reset() {
    buffer.clear();
    sent = 0;
}

bool ClientHandler::SendBuffer::isFullySent() const {
    return buffer.empty() || sent >= buffer.size();
}

ClientHandler::SendBuffer::SendStatus ClientHandler::SendBuffer::send(int clientFd) {
    if (isFullySent())
        return SEND_DONE;

    size_t bytes_to_send = buffer.size() - sent;
    ssize_t bytes_sent = ::send(clientFd, buffer.data() + sent, bytes_to_send, 0);

    if (bytes_sent < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return SEND_AGAIN;
        }
        LOG_SERROR(strerror(errno));
        return SEND_ERROR;
    }

    sent += bytes_sent;

    if (sent == buffer.size()) {
        reset();
        return SEND_DONE;
    }
    return SEND_AGAIN;
}

ClientHandler::CgiState::CgiState() : handler(NULL), isDone(false) {}

void ClientHandler::CgiState::clear() {
    handler = NULL;
    isDone = false;
}
void ClientHandler::CgiState::remove() {
    LOG_SDEBUG("starting removal. Handler: " << (handler ? "present" : "NULL"));
    if (!handler)
        return;
    EventDispatcher::getInstance().removeHandler(handler);
    LOG_SDEBUG("CGI handler removed from dispatcher.");
    clear();
}

// =============================================================================
// ClientHandler Implementation
// =============================================================================

ClientHandler::ClientHandler(int clientFd, int port, std::string const &clientAddr,
                             http::Router const &router)
    : clientFd_(clientFd),
      port_(port),
      clientAddr_(clientAddr),
      headersSent_(false),
      router_(router),
      reqParser_(request_, IO_BUFFER_SIZE),
      isKeepAlive_(false),
      rspBuffer_(IO_BUFFER_SIZE) {

    resetForNewRequest();
    LOG_INFO("ClientHandler(" << clientFd_ << "): connected from " << clientAddr_);
}

ClientHandler::~ClientHandler() {
    LOG_SDEBUG("destroying handler for fd=" << clientFd_);
    if (clientFd_ >= 0) {
        ::close(clientFd_);
        clientFd_ = -1;
    }
    cgiState_.remove();
}

int ClientHandler::getFd() const { return clientFd_; }

void ClientHandler::handleEvent(uint32_t events) {
    if (events & (EPOLLHUP | EPOLLERR)) {
        LOG_STRACE("EPOLLHUP or EPOLLERR received. Closing.");
        closeConnection();
        return;
    }
    if (events & EPOLLIN) {
        handleRead();
    }
    if (events & EPOLLOUT) {
        if (cgiState_.handler)
            handleCgiResponseWrite();
        else
            handleStaticResponseWrite();
    }
}

// =============================================================================
// Reading & Parsing Logic
// =============================================================================

void ClientHandler::handleRead() {
    char buffer[IO_BUFFER_SIZE];
    ssize_t count = ::recv(clientFd_, buffer, IO_BUFFER_SIZE, 0);

    if (count <= 0) {
        if (count < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            LOG_ERROR("ClientHandler::read(" << clientFd_ << "): error " << strerror(errno));
        }
        closeConnection();
        return;
    }

    handleRequestParsingState(reqParser_.feed(buffer, static_cast<size_t>(count)));
}

void ClientHandler::handleRequestParsingState(http::RequestParser::State state) {
    switch (state) {
    case http::RequestParser::ERROR:
        LOG_SWARN("Request parsing failed with status: " << reqParser_.errorStatus());
        handleError(reqParser_.errorStatus());
        break;

    case http::RequestParser::HEADERS_READY:
        router_.matchServerAndLocation(port_, request_);
        handleRequestParsingState(reqParser_.proceedReadingBody());
        break;

    case http::RequestParser::REQUEST_READY:
        router_.matchServerAndLocation(port_, request_);
        {
            std::string connectionHeader = request_.headers().get("Connection");
            utils::toLower(connectionHeader);
            if (connectionHeader == "keep-alive") {
                isKeepAlive_ = true;
            } else if (connectionHeader == "close") {
                isKeepAlive_ = false;
            } else {
                isKeepAlive_ = (request_.version() == "HTTP/1.1");
            }
        }
        generateResponse();
        break;

    default:
        break; // WAITING_FOR_DATA
    }
}

// =============================================================================
// Response Generation
// =============================================================================

void ClientHandler::generateResponse() {
    try {
        request_.remoteAddr(clientAddr_);
        router_.dispatch(port_, request_, response_);
    } catch (std::exception const &e) {
        LOG_SERROR("Dispatch exception: " << e.what());
        return handleError(http::INTERNAL_SERVER_ERROR);
    } catch (...) {
        LOG_SERROR("Unknown dispatch exception.");
        return handleError(http::INTERNAL_SERVER_ERROR);
    }

    http::IResponseBody *body = response_.body();

    if (body && !body->isDone() && body->getEventSourceFd() != -1) {
        if (!setupCgiHandler(body)) {
            handleError(http::INTERNAL_SERVER_ERROR);
        }
    } else {
        setupStaticResponse();
    }
}

bool ClientHandler::setupCgiHandler(http::IResponseBody *body) {
    LOG_SDEBUG("starting CGI handler");

    cgiState_.handler = NULL;
    try {
        cgiState_.handler = new CGIHandler(*body, *this, body->hasHeaderParsing());
        EventDispatcher::getInstance().registerHandler(cgiState_.handler);
        EventDispatcher::getInstance().disableRead(this);
        LOG_STRACE("CGI handler registered.");
        return true;
    } catch (std::exception const &e) {
        LOG_SERROR("Failed to setup CGI: " << e.what());
        delete cgiState_.handler;
        cgiState_.handler = NULL;
        return false;
    }
}

void ClientHandler::setupStaticResponse() {
    LOG_STRACE("preparing static response");
    cgiState_.handler = NULL;

    response_.headers().add("Connection", isKeepAlive_ ? "keep-alive" : "close");

    if (!response_.body() && !response_.headers().has("Content-Length")) {
        http::HttpStatus s = response_.status();
        if (s != http::NO_CONTENT && s != http::NOT_MODIFIED && s >= 200) {
            response_.headers().add("Content-Length", "0");
        }
    }

    response_.buildHeaders(rspBuffer_.buffer);
    headersSent_ = true;

    EventDispatcher::getInstance().enableWrite(this);
}

// =============================================================================
// Response Writing (Passive / Static)
// =============================================================================

void ClientHandler::handleStaticResponseWrite() {
    SendBuffer::SendStatus status = rspBuffer_.send(clientFd_);
    if (status == SendBuffer::SEND_ERROR)
        return closeConnection();
    if (status == SendBuffer::SEND_AGAIN)
        return;

    http::IResponseBody *body = response_.body();
    if (!body || body->isDone()) {
        return finalizeConnection();
    }

    rspBuffer_.buffer.resize(rspBuffer_.buffer.capacity());
    ssize_t bytesRead = body->read(rspBuffer_.buffer.data(), rspBuffer_.buffer.capacity());

    if (bytesRead <= 0) {
        return finalizeConnection();
    }

    rspBuffer_.buffer.resize(bytesRead);

    status = rspBuffer_.send(clientFd_);
    if (status == SendBuffer::SEND_ERROR)
        return closeConnection();
}

// =============================================================================
// Response Writing (Active / CGI)
// =============================================================================

void ClientHandler::pushToSendBuffer(const char *data, size_t length) {
    bool wasEmpty = rspBuffer_.buffer.empty();
    rspBuffer_.buffer.insert(rspBuffer_.buffer.end(), data, data + length);

    if (wasEmpty) {
        EventDispatcher::getInstance().enableWrite(this);
    }
    if (isSendBufferFull())
        EventDispatcher::getInstance().disableRead(cgiState_.handler);
}

bool ClientHandler::isSendBufferFull() const { return rspBuffer_.buffer.size() > (1024 * 1024); }

void ClientHandler::onCgiComplete() {
    LOG_SDEBUG("CGI completion signaled.");
    cgiState_.isDone = true;

    if (cgiState_.handler) {
        cgiState_.remove();
    }

    if (rspBuffer_.isFullySent()) {
        finalizeConnection();
    } else {
        EventDispatcher::getInstance().enableWrite(this);
    }
}

void ClientHandler::onCgiHeadersParsed(http::Headers const &headers) {
    response_.headers() = headers;

    http::Headers::const_iterator it = headers.find("Status");
    if (it != headers.end()) {
        http::HttpStatus st = http::toHttpStatus(it->second);
        if (st != http::UNKNOWN_STATUS)
            response_.status(st);
        response_.headers().erase("Status");
    }

    response_.headers().add("Connection", isKeepAlive_ ? "keep-alive" : "close");
    response_.buildHeaders(rspBuffer_.buffer, true);
    headersSent_ = true;
    LOG_SDEBUG("Headers built, activating send.");

    EventDispatcher::getInstance().enableWrite(this);
}

void ClientHandler::handleCgiResponseWrite() {
    SendBuffer::SendStatus status = rspBuffer_.send(clientFd_);

    if (status == SendBuffer::SEND_ERROR) {
        closeConnection();
        return;
    }

    if (rspBuffer_.isFullySent()) {
        if (cgiState_.isDone || !cgiState_.handler) {
            finalizeConnection();
        } else {
            LOG_SDEBUG("Send buffer empty. Pausing ClientHandler.");
            EventDispatcher::getInstance().disableWrite(this);
            EventDispatcher::getInstance().enableRead(cgiState_.handler);
        }
    }
}

// =============================================================================
// Error & Lifecycle Management
// =============================================================================

void ClientHandler::handleError(http::HttpStatus status) {
    LOG_SWARN("Handling error: " << status);
    if (headersSent_) {
        LOG_SWARN("Headers already sent, cannot send error page. Closing connection.");
        closeConnection();
        return;
    }

    if (cgiState_.handler) {
        cgiState_.remove();
    }

    rspBuffer_.reset();
    response_.clear();
    response_.status(status);

    router_.handleError(request_, response_);

    response_.headers().add("Connection", isKeepAlive_ ? "keep-alive" : "close");
    response_.buildHeaders(rspBuffer_.buffer);
    headersSent_ = true;

    EventDispatcher::getInstance().enableWrite(this);
}

void ClientHandler::finalizeConnection() {
    if (isKeepAlive_) {
        LOG_SDEBUG("Keep-Alive. Resetting.");
        resetForNewRequest();
        EventDispatcher::getInstance().enableRead(this);
        EventDispatcher::getInstance().disableWrite(this);
    } else {
        closeConnection();
    }
}

void ClientHandler::closeConnection() {
    LOG_SDEBUG("Initiating connection closure.");
    cgiState_.remove();
    EventDispatcher::getInstance().removeHandler(this);
}

void ClientHandler::resetForNewRequest() {
    LOG_SDEBUG("Resetting all state.");
    cgiState_.remove();
    rspBuffer_.reset();
    reqParser_.reset();
    response_.clear();
    request_.clear();
    headersSent_ = false;
}

std::string ClientHandler::getLogSignature() const {
    return "[fd:" + utils::toString(clientFd_) + "]";
}

} // namespace network
