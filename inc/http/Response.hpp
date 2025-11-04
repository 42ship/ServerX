#pragma once

#include "http/Headers.hpp"
#include "http/HttpStatus.hpp"
#include <vector>

namespace http {

// Forward-declare the body interface
class IResponseBody;

/**
 * @class ResponseStartLine
 * @brief A simple data aggregate for the HTTP response status line.
 */
class ResponseStartLine {
public:
    ResponseStartLine();

    std::string protocol;      //!< The HTTP protocol version (e.g., "HTTP/1.1").
    HttpStatus statusCode; //!< The numerical status code (e.g., 200, 404).
    std::string reasonPhrase;  //!< The human-readable reason (e.g., "OK", "Not Found").
};

/**
 * @class Response
 * @brief A data container for an HTTP response, managed via a fluent API.
 *
 * This class encapsulates all components of an HTTP response. Its state is
 * modified by handlers through a fluent interface (methods returning *this),
 * which ensures resources are managed safely and state is kept consistent.
 */
class Response {
public:
    Response();
    ~Response();

    /**
     * @brief Resets the response to a default state, ready for reuse.
     * Cleans up any existing body and resets headers and status.
     * @return A reference to this object for chaining.
     */
    Response &clear();

    // --- Body Management ---

    /**
     * @brief Gets a pointer to the (read-only) response body.
     * @return A const pointer to the response body, or NULL if no body is set.
     */
    IResponseBody *body() const;

    /**
     * @brief Sets the response to have no body.
     * Cleans up any existing body.
     * @return A reference to this object for chaining.
     */
    Response &setNoBody();

    /**
     * @brief Sets the response body from an in-memory string.
     * This takes ownership of the data (by copying) and sets Content-Type.
     * @param data The string data to use as the body.
     * @param contentType The MIME type of the body (e.g., "application/json").
     * @return A reference to this object for chaining.
     */
    Response &setBodyInMemory(std::string const &data, std::string const &contentType);

    /**
     * @brief Sets the response body to stream from a file.
     * Creates a FileBody object to manage the file descriptor.
     * @param fpath The file path to read from.
     * @param contentType The MIME type of the file.
     * @return A reference to this object for chaining.
     */
    Response &setBodyFromFile(std::string const &fpath, std::string const &contentType);

    /**
     * @brief Sets the response body to stream from a CGI pipe.
     * @param pipe_fd The file descriptor of the CGI script's output pipe.
     * @param firstChunk A small chunk of data already read from the pipe (for headers).
     * @return A reference to this object for chaining.
     */
    Response &setBodyFromCgi(int pipe_fd, std::string const &firstChunk);

    // --- Header & Status Management ---

    /**
     * @brief Gets a mutable reference to the response headers.
     */
    Headers &headers();

    /**
     * @brief Gets a constant reference to the response headers.
     */
    Headers const &headers() const;

    /**
     * @brief Appends the fully formatted start-line and headers to a buffer.
     * This is a high-performance method that avoids temporary string creation.
     * @param buffer The send buffer to which headers will be appended.
     */
    void buildHeaders(std::vector<char> &buffer) const;

    /**
     * @brief Sets the response status and automatically syncs the reason phrase.
     * @param statusCode The HttpStatus enum value (e.g., NOT_FOUND).
     * @return A reference to this object for chaining.
     */
    Response &status(HttpStatus statusCode);

    /** @copydoc status(HttpStatus statusCode) */
    Response &status(HttpStatus statusCode, std::string const &customError);

    /**
     * @brief Gets the current status code.
     */
    HttpStatus status() const;

    /**
     * @brief Gets the current protocol string (e.g., "HTTP/1.1").
     */
    std::string const &protocol() const;

    /**
     * @brief Gets the current reason phrase (e.g., "Not Found").
     */
    std::string const &reasonPhrase() const;

private:
    Response(Response const &);
    Response const &operator=(Response const &);

    /**
     * @brief Internal: safely deletes the current body object.
     */
    void cleanupBody_();

    /**
     * @brief Internal: maps a Status enum to its standard string.
     */
    static char const *getReasonPhrase_(HttpStatus status);

    ResponseStartLine startLine_;
    Headers headers_;
    IResponseBody *body_;
    std::string customError_;
};

} // namespace http
