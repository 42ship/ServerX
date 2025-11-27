#include "network/CGIHandler.hpp"
#include "http/HttpStatus.hpp"
#include "http/ResponseBody.hpp"
#include "network/EventDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cstring>
#include <sys/epoll.h>

namespace network {

static const size_t MAX_CGI_HEADER_SIZE = 8192;

CGIHandler::CGIHandler(http::IResponseBody &body, ClientHandler &client, bool isNoParseHeaders)
    : body_(body), client_(client), isNPH_(isNoParseHeaders), fd_(body.getEventSourceFd()) {
    if (isNPH_) {
        state_ = STREAMING_BODY;
    } else {
        state_ = READING_HEADERS;
    }
}

int CGIHandler::getFd() const { return fd_; }

void CGIHandler::handleRead() {
    LOG_TRACE("CGIHandler::handleRead(" << client_.getFd() << ")");
    if (state_ == STREAMING_BODY && client_.isSendBufferFull()) {
        LOG_DEBUG("Client send buffer is full. Disabling CGI read handler temporarily.");
        EventDispatcher::getInstance().modifyHandler(this, 0);
    }

    char buffer[8192];
    ssize_t bytes_read = body_.read(buffer, sizeof(buffer));
    LOG_TRACE("CGIHandler::handleRead: Read " << bytes_read << " bytes from CGI body pipe.");
    if (bytes_read <= 0) {
        if (state_ == READING_HEADERS) {
            LOG_WARN("CGIHandler::handleRead: CGI process ended or failed before sending valid "
                     "headers. Sending 502 Bad "
                     "Gateway.");
            // TODO: Kill CGI process
            client_.handleError(http::BAD_GATEWAY);
        } else {
            LOG_DEBUG("CGIHandler::handleRead: CGI body stream finished. Completing request.");
            client_.onCgiComplete();
        }
    }
    if (state_ == STREAMING_BODY) {
        LOG_TRACE("CGIHandler::handleRead: State is STREAMING_BODY. Pushing "
                  << bytes_read << " bytes to client buffer.");
        return client_.pushToSendBuffer(buffer, bytes_read);
    }
    if (headerBuffer_.length() + bytes_read > MAX_CGI_HEADER_SIZE) {
        LOG_ERROR("CGIHandler::handleRead: CGI headers exceeded maximum allowed size ("
                  << MAX_CGI_HEADER_SIZE << "). Sending 502.");
        // TODO: Kill CGI process
        return client_.handleError(http::BAD_GATEWAY);
    }
    headerBuffer_.append(buffer, bytes_read);
    LOG_SERROR(headerBuffer_);
    size_t headerEnd = headerBuffer_.find("\r\n\r\n");
    size_t offset = 4; // Length of \r\n\r\n

    if (headerEnd == std::string::npos) {
        headerEnd = headerBuffer_.find("\n\n");
        offset = 2; // Length of \n\n
    }

    if (headerEnd == std::string::npos) {
        headerEnd = headerBuffer_.find("\n\r\n");
        offset = 3; // Length of \n\r\n
    }
    if (headerEnd == std::string::npos) {
        return;
    }
    LOG_DEBUG("CGIHandler::handleRead: Found end of CGI headers. Parsing headers...");
    http::Headers headers;
    if (!http::Headers::parse(headerBuffer_, headers, false)) {
        LOG_ERROR("CGIHandler::handleRead: Failed to parse CGI headers. Malformed response. "
                  "Sending 502.");
        // TODO: Kill CGI process
        return client_.handleError(http::BAD_GATEWAY);
    }
    LOG_DEBUG("CGIHandler::handleRead: CGI headers parsed successfully. Forwarding to client.");
    client_.onCgiHeadersParsed(headers);
    state_ = STREAMING_BODY;
    if (headerEnd + offset < headerBuffer_.length()) {
        size_t bodyBytes = headerBuffer_.length() - (headerEnd + offset);
        LOG_DEBUG("CGIHandler::handleRead: Pushing remaining "
                  << bodyBytes << " bytes of body found in header buffer.");
        client_.pushToSendBuffer(headerBuffer_.data() + headerEnd + offset, headerBuffer_.length());
    }
}

void CGIHandler::handleEvent(uint32_t events) {
    if (events & EPOLLIN)
        handleRead();
    else {
        LOG_TRACE("CGIHandler::handleEvent(" << client_.getFd() << "): " << events);
        client_.onCgiComplete();
    }
}

#if 0
void CGIHandler::handleEvent(uint32_t events) {
    if (!(events & (EPOLLIN | EPOLLHUP | EPOLLERR)))
        return;

    LOG_TRACE("CGIHandler::handleEvent(" << client_.getFd() << ")");

    char buffer[8192];

    while (true) {
        if (client_.isSendBufferFull()) {
            LOG_DEBUG("CGIHandler::handleEvent("
                      << client_.getFd()
                      << "): client buffer is full. Pausing CGI and waking client.");
            EventDispatcher::getInstance().modifyHandler(this, 0);
            EventDispatcher::getInstance().setSendingData(&client_);
            return;
        }
        ssize_t bytes_read = body_.read(buffer, sizeof(buffer));
        if (bytes_read > 0) {
            LOG_TRACE("CGIHandler::handleEvent(" << client_.getFd() << ")::read: " << bytes_read
                                                 << " bytes");
            client_.pushToSendBuffer(buffer, bytes_read);
        } else if (bytes_read == 0) {
            LOG_DEBUG("CGIHandler::handleEvent(" << client_.getFd()
                                                 << "): all data read successfully (EOF)");
            client_.onCgiComplete();
            return;

        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                LOG_TRACE("CGIHandler::handleEvent(" << client_.getFd()
                                                     << "): pipe is empty, will try again later.");
                break;
            }
            LOG_ERROR("CGIHandler::handleEvent: " << strerror(errno));
            client_.onCgiComplete();
            return;
        }
    }
    EventDispatcher::getInstance().modifyHandler(this, 0);
    EventDispatcher::getInstance().setSendingData(&client_);
}
#endif

} // namespace network
