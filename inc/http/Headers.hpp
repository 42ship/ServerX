#pragma once

#include <map>
#include <string>

namespace http {

class Headers {
public:
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

    std::string get(const std::string &key) const;

    /**
     * @brief A specialized accessor for the Content-Length header.
     * @return The value of Content-Length as a number, or 0 if not present or
     * invalid.
     */
    size_t getConentLength() const;

    bool isContentChunked() const;

    Headers &clear();

    Headers &erase(std::string const &key);

    /// @brief Parses a raw string into a Headers object.
    static Headers parse(std::string &);

    /// @brief Parses a stream of text into a Headers object.
    static Headers parse(std::istringstream &);

private:
    typedef std::map<std::string, std::string> HeaderMap;

    static void normalizeKey(std::string &key);
    static std::string normalizeKey(std::string const &key);

    HeaderMap map_;
};

} // namespace http
