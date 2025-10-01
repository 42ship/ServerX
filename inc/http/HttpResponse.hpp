#pragma once

#include <vector>
#include <map>
#include <string>
#include <ostream>
#include "HttpStatus.hpp"

namespace http {

typedef std::map<std::string, std::string> HeaderMap;

/**
 * @enum BodySourceType
 * @brief Defines the source of the data for an HTTP response body.
 *
 * This allows the server's Reactor to determine how to handle the sending
 * of the response body, whether it's from memory, a file, or another process.
 */
enum BodySourceType {
    BODY_NONE,      /**< No body will be sent (e.g., for HEAD requests or 204 No Content). */
    BODY_IN_MEMORY, /**< The body is fully generated and stored in a memory buffer. */
    BODY_FROM_FILE, /**< The body should be streamed directly from a file descriptor. */
    BODY_FROM_CGI   /**< The body should be streamed from a CGI script's output pipe. */
};

/**
 * @enum MessageType
 * @brief Defines the format of the HTTP response status message.
 */
enum MessageType {
    STANDARD, ///< Use standard HTTP reason phrase (e.g., "OK", "Not Found")
    JSON      ///< Use JSON-formatted error message
};

/**
 * @class HttpResponse
 * @brief A data container for an HTTP response.
 *
 * This object is populated by a handler and then serialized into a raw
 * HTTP response string by the ResponseBuilder.
 */
class HttpResponse {
public:
    HttpResponse();
    HttpResponse(Status code, std::string const &httpVersion, MessageType type = STANDARD);
    HttpResponse(HttpResponse const &);
    HttpResponse const &operator=(HttpResponse const &);
    ~HttpResponse();

    void setStatus(Status s);
    void setNoBody();
    void setBodyInMemory(std::vector<char> const &, std::string const &mimeType);
    void setBodyFromFile(int fd, size_t size);
    void setBodyFromFile(int fd, size_t size, std::string const &mimeType);
    void setBodyFromCgi(int pipe_fd);

    Status getStatus() const;
    // char const *getResponsePhrase() const;
    HeaderMap &getHeaders();
    HeaderMap const &getHeaders() const;
    std::string const &getVersion() const;
    BodySourceType getBodyType() const;
    std::string buildHeaders() const;
    MessageType getMessageType() const;

    char const *generateResponsePhrase() const;

    union {
        /** @brief Details for in-memory bodies. */
        struct {
            std::vector<char> *data; /**< Pointer to the internal buffer holding the body. */
            size_t sent;             /**< Tracks bytes sent by the Reactor. */
        } inMemoryBody;

        /** @brief Details for file-based bodies. */
        struct {
            int fd;           /**< File descriptor to read from. */
            size_t totalSize; /**< Total size of the file. */
            size_t sent;      /**< Tracks bytes sent by the Reactor. */
        } fileBody;

        /** @brief Details for CGI-based bodies. */
        struct {
            int pipe_fd; /**< Pipe descriptor to read from. */
        } cgiBody;
    };

private:
    std::string httpVersion_;
    Status statusCode_;
    HeaderMap headers_;
    BodySourceType bodyType_;
    MessageType messageType_;

    void cleanupBody();
};

std::ostream &operator<<(std::ostream &o, HttpResponse const &r);

} // namespace http
