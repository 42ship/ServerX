#pragma once

#include <map>
#include <sstream>
#include <string>

namespace http {

/**
 * @class Headers
 * @brief Manages a collection of HTTP headers.
 *
 * This class stores HTTP headers in a case-insensitive manner
 * (by normalizing keys to lowercase) and provides helpers for
 * common header operations.
 */
class Headers {
public:
    typedef std::map<std::string, std::string> HeaderMap;

    /**
     * @brief Constructs an empty Headers object.
     */
    Headers();

    /**
     * @brief Checks if a header with the given key exists.
     * @param key The header name (case-insensitive).
     * @return True if the header is present, false otherwise.
     */
    bool has(std::string const &key) const;

    /**
     * @brief Adds or overwrites a header key-value pair.
     * @param key The header name.
     * @param value The header value.
     * @return A reference to this Headers object for chaining.
     */
    Headers &add(std::string const &key, std::string const &value);

    /**
     * @brief C++98-style getter. Retrieves a header value.
     * @param key The header name (case-insensitive).
     * @param[out] value The string to be filled with the header's value.
     * @return True if the key was found, false otherwise.
     */
    bool get(const std::string &key, std::string &value) const;

    /**
     * @brief C++11-style getter. Retrieves a header value.
     * @param key The header name (case-insensitive).
     * @return The header value as a string. Returns an empty string if not found.
     */
    std::string get(const std::string &key) const;

    /**
     * @brief A specialized accessor for the Content-Length header.
     * @return The value of Content-Length as a number, or 0 if not present or
     * invalid.
     */
    size_t getContentLength() const;

    /**
     * @brief Checks if the 'Transfer-Encoding' header is set to 'chunked'.
     * @return True if the transfer encoding is chunked, false otherwise.
     */
    bool isContentChunked() const;

    /**
     * @brief Clears all headers from the map.
     * @return A reference to this Headers object for chaining.
     */
    Headers &clear();

    /**
     * @brief Removes a single header by its key.
     * @param key The header name to remove (case-insensitive).
     * @return A reference to this Headers object for chaining.
     */
    Headers &erase(std::string const &key);

    /**
     * @brief Serializes all headers into a single string.
     * Each header is on its own line, formatted as "Key: Value\r\n".
     * @return A string containing all headers for an HTTP response.
     */
    std::string toString() const;

    /**
     * @brief Parses a raw string of headers into a Headers object.
     * The string should contain multiple "Key: Value\r\n" lines.
     */
    static Headers parse(std::string &);

    /**
     * @brief Parses a stream of text into a Headers object.
     * The stream should contain multiple "Key: Value\r\n" lines.
     */
    static Headers parse(std::istringstream &);

    HeaderMap::const_iterator begin() const;
    HeaderMap::const_iterator end() const;

private:
    /**
     * @brief Internal: Normalizes a key to lowercase in-place.
     * @internal
     */
    static void normalizeKey(std::string &key);

    /**
     * @brief Internal: Returns a lowercase version of a key.
     * @internal
     */
    static std::string normalizeKey(std::string const &key);

    /**
     * @brief The internal storage for headers, mapping
     * lowercase_key -> Original_Value.
     * @internal
     */
    HeaderMap map_;
};

} // namespace http
