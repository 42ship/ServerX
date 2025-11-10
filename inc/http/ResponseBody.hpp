#pragma once

#include <aio.h>
#include <string>

namespace http {

/**
 * @class IResponseBody
 * @brief An abstract interface for all HTTP response body sources.
 *
 * This class defines a "strategy" for sending a response body. It allows
 * the Reactor to stream data from any source (memory, file, CGI pipe)
 * by simply calling read(), without needing to know the underlying type.
 */
class IResponseBody {
public:
    IResponseBody() {}
    virtual ~IResponseBody() {}

    /**
     * @brief Reads a chunk of the body into the provided buffer.
     * This is a non-blocking operation.
     *
     * @param buffer An output buffer to write data into.
     * @param size The maximum size of the `buffer`.
     * @return The number of bytes read ( > 0).
     * @return 0 on End-of-File (EOF).
     * @return -1 on error. `errno` may be set (e.g., to EAGAIN/EWOULDBLOCK
     * if the source is not ready).
     */
    virtual ssize_t read(char *buffer, size_t size) = 0;

    /**
     * @brief Returns the total size of the body in bytes, if known.
     *
     * @return The total content length. Returns 0 or (size_t)-1
     * if the size is unknown (e.g., for a streaming CGI response).
     */
    virtual size_t size() const = 0;

    /**
     * @brief Checks if the body source has been fully read.
     *
     * @return true if the source is depleted (EOF has been reached),
     * false otherwise.
     */
    virtual bool isDone() const = 0;

    /**
     * @brief Returns the file descriptor for this body, if it has one.
     *
     * This method tells the ClientHandler if this body is an "active"
     * (event-driven) source or a "passive" (in-memory) one.
     *
     * @return A valid file descriptor (>= 0) if the source needs to be
     * watched by epoll.
     * @return -1 if the source is passive (e.g., a std::string) and
     * has no associated file descriptor.
     */
    virtual int getEventSourceFd() const = 0;

private:
    IResponseBody(IResponseBody const &);
    IResponseBody const &operator=(IResponseBody const &);
};

class NoBody : public IResponseBody {
public:
    ssize_t read(char *, size_t);
    size_t size() const;
    bool isDone() const;
    int getEventSourceFd() const;
};

class FileBody : public IResponseBody {
public:
    FileBody(std::string const &fpath);
    ~FileBody();

    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;
    int getEventSourceFd() const;

private:
    int fd_;
    size_t size_;
    size_t sent_;
};

class BodyInMemory : public IResponseBody {
public:
    BodyInMemory(std::string const &);

    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;
    int getEventSourceFd() const;

private:
    std::string body_;
    size_t bytesRead_;
};

class BodyFromCgi : public IResponseBody {
public:
    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;
    int getEventSourceFd() const;

private:
    int fd_;
};

} // namespace http
