#pragma once

#include "http/Headers.hpp"
#include "http/HttpStatus.hpp"
#include <aio.h>

namespace config {
class LocationBlock;
class ServerBlock;
} // namespace config

namespace network {
class ClientHandler;
} // namespace network

namespace http {

/**
 * @class RequestStartLine
 * @brief Holds the parsed components of an HTTP request's first line.
 */
class RequestStartLine {
public:
    enum Method { GET, POST, PUT, DELETE, UNKNOWN };

    RequestStartLine();

    Method method;
    std::string uri;         //!< The full, original request-target (e.g., "/path?query=1")
    std::string path;        //!< The path portion only (e.g., "/path")
    std::string queryString; //!< The query string only (e.g., "query=1")
    std::string version;     //!< The HTTP protocol version (e.g., "HTTP/1.1")

    /**
     * @brief Parses a raw request line (e.g., "GET /path?q HTTP/1.1").
     * @param line The raw string to parse.
     * @return A populated RequestStartLine object.
     */
    static RequestStartLine parse(std::string const &line);

    /**
     * @brief Converts a method string (e.g., "GET") to its enum.
     */
    static Method matchHttpMethod(std::string const &s);

    /**
     * @brief Converts a method enum to its string representation.
     */
    static char const *methodToString(Method m);
};

/**
 * @class HttpRequest
 * @brief A data container for an HTTP request, acting as a "request context".
 *
 * This class encapsulates all components of an incoming HTTP request.
 * It is populated by the `RequestParser` (a friend class) and then
 * enriched by the `Router` (also a friend) with configuration details
 * like the matched server and location blocks.
 */
class Request {
public:
    Request();
    // ~HttpRequest(); // Will be needed to manage body_

    /**
     * @brief Checks the 'Accept' header to see if the client prefers JSON.
     * @return True if 'application/json' is found in the Accept header.
     */
    bool wantsJson() const;

    /**
     * @brief Resets the request to a default state, ready for reuse.
     * Cleans up any existing body and resets all fields.
     */
    void clear();

    size_t getMaxAllowedContentSize() const;

    // --- Public Read-only Accessors ---

    /**
     * @brief Gets a const pointer to the matched LocationBlock.
     * @return Const pointer to the block, or NULL if not matched.
     */
    config::LocationBlock const *location() const;

    /**
     * @brief Gets a const pointer to the matched ServerBlock.
     * @return Const pointer to the block, or NULL if not matched.
     */
    config::ServerBlock const *server() const;

    /**
     * @brief Gets a mutable reference to the request headers.
     * @note This is public to allow handlers to inspect headers.
     */
    Headers &headers();

    /**
     * @brief Gets a constant reference to the request headers.
     */
    Headers const &headers() const;

    // --- RequestLine Accessors ---

    /**
     * @brief Gets the HTTP method enum.
     */
    RequestStartLine::Method method() const;

    /**
     * @brief Gets the full, original request URI (e.g., "/path?query=1").
     */
    std::string const &uri() const;

    Request &uri(std::string const &);

    /**
     * @brief Gets the path portion of the URI (e.g., "/path").
     */
    std::string const &path() const;

    /**
     * @brief Gets the query string portion of the URI (e.g., "query=1").
     */
    std::string const &queryString() const;

    /**
     * @brief Gets the HTTP protocol version (e.g., "HTTP/1.1").
     */
    std::string const &version() const;

    int body() const;

    Request &body(int fd);

    /**
     * @brief Sets the response status and automatically syncs the reason phrase.
     * @param statusCode The HttpStatus enum value (e.g., NOT_FOUND).
     * @return A reference to this object for chaining.
     */
    Request &status(HttpStatus statusCode);

    /**
     * @brief Gets the current status code.
     */
    HttpStatus status() const;

    /**
     * @brief Resolves the full filesystem path for a request.
     *
     * This function correctly implements the logic for both the Nginx 'root'
     * and 'alias' directives. 'alias' takes precedence if it exists.
     *
     * @return The fully resolved filesystem path.
     */
    std::string resolvePath() const;

    /**
     * @brief Gets the remote (client) IP address for this request.
     * @return The client IP address as a string (e.g., "192.168.1.100").
     */
    std::string const &remoteAddr() const;

    // ======================== Test Helpers (Public API) ========================

    /**
     * @brief Test helper: Sets the location block for this request.
     * @note This is a public method specifically for testing purposes.
     * @param location Pointer to the LocationBlock to associate with this request.
     * @return A reference to this Request object for chaining.
     */
    Request &setLocationBlock(config::LocationBlock const *location);

    /**
     * @brief Test helper: Sets the server block for this request.
     * @note This is a public method specifically for testing purposes.
     * @param server Pointer to the ServerBlock to associate with this request.
     * @return A reference to this Request object for chaining.
     */
    Request &setServerBlock(config::ServerBlock const *server);

protected:
    friend class RequestParser;
    friend class Router;
    friend class network::ClientHandler;

    Request &location(config::LocationBlock const *location);
    Request &server(config::ServerBlock const *server);
    Request &method(RequestStartLine::Method method);
    Request &version(std::string const &);
    Request &remoteAddr(std::string const &addr);

private:
    RequestStartLine requestLine_;
    Headers headers_;
    int body_;
    config::LocationBlock const *location_;
    config::ServerBlock const *server_;
    HttpStatus status_;
    std::string remoteAddr_;  // Client IP address
};

} // namespace http
