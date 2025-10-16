#pragma once

#include <aio.h>
#include <string>

namespace http {

struct Body {
public:
    enum BodyType {
        BODY_NONE,      /**< No body will be sent (e.g., for HEAD requests or 204 No Content). */
        BODY_IN_MEMORY, /**< The body is fully generated and stored in a memory buffer. */
        BODY_FROM_FILE, /**< The body should be streamed directly from a file descriptor. */
        BODY_FROM_CGI   /**< The body should be streamed from a CGI script's output pipe. */
    };

    Body();

    void setNoBody();
    void setBodyFromString(std::string &data);
    void setBodyFromFile(int fd, size_t size);
    void setBodyFromCgi(int pipe_fd);
    BodyType getBodyType() const;

    union {
        struct {
            std::string *data; /**< Pointer to the internal buffer holding the body. */
            size_t sent;       /**< Tracks bytes sent by the Reactor. */
        } inMemoryBody;
        struct {
            int fd;           /**< File descriptor to read from. */
            size_t totalSize; /**< Total size of the file. */
            size_t sent;      /**< Tracks bytes sent by the Reactor. */
        } fileBody;
        struct {
            int pipe_fd; /**< Pipe descriptor to read from. */
        } cgiBody;
    };

private:
    BodyType type_;
};

} // namespace http
