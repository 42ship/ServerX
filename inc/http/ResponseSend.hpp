#pragma once

#include <string>
#include <sys/types.h>
#include "Response.hpp"

/**
 * @class ResponseSend
 * @brief Utility class for sending HTTP responses over a socket.
 * This class provides methods to send complete responses or parts of them
 * to a client socket.
 */
class ResponseSend {
public:
    /**
     * @brief Sends the entire data buffer to the client socket.
     *
     * @param clientFd File descriptor of the client socket.
     * @param data     Data buffer to send.
     * @return True if the data was sent successfully, false otherwise.
     */
    static bool sendAll(int clientFd, const std::string &data);

    /**
     * @brief Sends part of the data buffer to the client socket.
     *
     * Useful for non-blocking sockets.
     *
     * @param clientFd File descriptor of the client socket.
     * @param data     Data buffer to send.
     * @return Number of bytes sent, or -1 on error.
     */
    static long sendSome(int clientFd, const std::string &data);
};
