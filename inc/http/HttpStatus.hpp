#pragma once

/**
 * @namespace HttpStatus
 * @brief Utilities for working with HTTP status codes and messages.
 */
namespace http {

/**
 * @enum Status
 * @brief Enumeration of common HTTP status codes.
 */
enum Status {
    OK = 200,         /** 200 OK — request succeeded. */
    CREATED = 201,    /** 201 Created — resource created. */
    ACCEPTED = 202,   /** 202 Accepted — processing pending. */
    NO_CONTENT = 204, /** 204 No Content — successful, no body. */

    BAD_REQUEST = 400,        /** 400 Bad Request — malformed syntax. */
    UNAUTHORIZED = 401,       /** 401 Unauthorized — authentication required/failed. */
    FORBIDDEN = 403,          /** 403 Forbidden — not allowed despite authentication. */
    NOT_FOUND = 404,          /** 404 Not Found — resource not found. */
    METHOD_NOT_ALLOWED = 405, /** 405 Method Not Allowed — method not permitted. */

    INTERNAL_SERVER_ERROR = 500 /** 500 Internal Server Error — server-side failure. */
};

/**
 * @brief Returns the canonical reason phrase for a given HTTP status code.
 *
 * @param status The HTTP status code.
 * @return A pointer to a null-terminated string with the reason phrase,
 *         e.g., "OK", "Not Found". Returns "Unknown" if the status
 *         is not recognized.
 */
inline const char *statusMessage(Status status) {
    switch (status) {
    case OK:
        return "OK";
    case CREATED:
        return "Created";
    case ACCEPTED:
        return "Accepted";
    case NO_CONTENT:
        return "No Content";
    case BAD_REQUEST:
        return "Bad Request";
    case UNAUTHORIZED:
        return "Unauthorized";
    case FORBIDDEN:
        return "Forbidden";
    case NOT_FOUND:
        return "Not Found";
    case METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case INTERNAL_SERVER_ERROR:
        return "Internal Server Error";
    default:
        return "Unknown";
    }
}

/**
 * @class StatusCode
 * @brief Lightweight wrapper around an HTTP @ref Status value with helpers.
 *
 * Provides access to the numeric status code and its reason phrase.
 */
class StatusCode {

public:
    /**
     * @brief Default constructor.
     * Initializes the status code to 200 OK.
     */
    StatusCode();
    /**
     * @brief Constructs a StatusCode from a @ref Status value.
     * @param code The HTTP status to store.
     */
    explicit StatusCode(Status code);

    /**
     * @brief Copy constructor.
     * @param other The instance to copy from.
     */
    StatusCode(const StatusCode &other);

    /**
     * @brief Copy assignment operator.
     * @param other The instance to copy from.
     * @return Reference to this instance.
     */
    StatusCode &operator=(const StatusCode &other);

    /**
     * @brief Returns the stored HTTP status code.
     * @return The @ref Status value.
     */
    Status getCode() const;

    /**
     * @brief Returns the canonical reason phrase for the stored code.
     * @return A pointer to a null-terminated string with the reason phrase.
     */
    const char *getMessage() const;

    /**
     * @brief Destructor.
     */
    ~StatusCode();

private:
    Status code_; /**< Stored HTTP status code. */
};

} // namespace http
