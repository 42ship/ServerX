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
     * @brief Retrieves the file descriptor for event-driven body sources.
     *
     * This method is used by the Reactor to determine if the response body
     * source requires monitoring via the event loop (epoll).
     *
     * @return A non-negative file descriptor (fd >= 0) if the source is "active"
     * (e.g., a CGI pipe) and must be watched for read events.
     * @return -1 if the source is "passive" (e.g., in-memory data or a standard file)
     * and does not require epoll monitoring.
     */
    virtual int getEventSourceFd() const;

    /**
     * @brief Indicates if the data read from this source contains HTTP headers
     * that need to be parsed by the server (e.g., CGI output).
     * * @return true if the Reactor should buffer and parse headers before streaming.
     * @return false if the data is the raw body (default).
     */
    virtual bool hasHeaderParsing() const;

private:
    IResponseBody(IResponseBody const &);
    IResponseBody const &operator=(IResponseBody const &);
};

class FileBody : public IResponseBody {
public:
    explicit FileBody(std::string const &fpath);
    ~FileBody();

    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;

private:
    int fd_;
    size_t size_;
    size_t sent_;
};

class BodyInMemory : public IResponseBody {
public:
    explicit BodyInMemory(std::string const &);

    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;

private:
    std::string body_;
    size_t bytesRead_;
};

class BodyFromCgi : public IResponseBody {
public:
    BodyFromCgi(int pipeFd, bool hasHeaderParsing);
    ~BodyFromCgi();
    ssize_t read(char *buffer, size_t size);
    size_t size() const;
    bool isDone() const;
    int getEventSourceFd() const;
    bool hasHeaderParsing() const;

private:
    BodyFromCgi();
    int fd_;
    bool isDone_;
    bool hasHeaderParsing_;
};

} // namespace http
