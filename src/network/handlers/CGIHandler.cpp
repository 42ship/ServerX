#include "network/CGIHandler.hpp"
#include "http/ResponseBody.hpp"
#include "network/EventDispatcher.hpp"
#include "utils/Logger.hpp"
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>

namespace network {

CGIHandler::CGIHandler(http::IResponseBody &body, ClientHandler &client, bool isNPH)
    : body_(body), client_(client), isNPH_(isNPH) {}

int CGIHandler::getFd() const { return body_.getEventSourceFd(); }

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

} // namespace network
