#pragma once

#include <cstddef>
#include <string>
#include "MimeTypes.hpp"

#define MIME_TYPES_PATH "config/mime.types"

namespace http {
/**
 * @class ResponseContent
 * @brief Encapsulates the body content and MIME type of an HTTP response.
 *
 * This class stores both the content (body) and the corresponding MIME type
 * for HTTP responses. It provides constructors for initializing the response
 * from a file path, from a raw body and MIME type, or by copying another
 * instance. It also provides accessors for retrieving the stored data and
 * utility methods for determining the MIME type from a file extension.
 */
class ResponseContent {
public:
    /**
     * @brief Default constructor.
     *
     * Initializes an empty response body and sets the MIME type to "text/plain".
     */
    ResponseContent();

    /**
     * @brief Constructs the response from a file.
     *
     * Reads the file at the given path, stores its content in the body,
     * and determines its MIME type by calling setFileType().
     *
     * @param path Path to the file to be read.
     * @param mime Pointer to mime tipes object created from @ref MIME_TYPES_PATH file
     */
    ResponseContent(const char *path, MimeTypes *mime);

    /**
     * @brief Constructs the response from raw body content and MIME type.
     *
     * @param body Body content to store.
     * @param type MIME type to associate with the body.
     *
     */
    ResponseContent(const std::string &body, const std::string &type);

    /**
     * @brief Copy constructor.
     *
     * Creates a new instance as a copy of another ResponseContent object.
     *
     * @param other Instance to copy from.
     */
    ResponseContent(const ResponseContent &other);

    /**
     * @brief Copy assignment operator.
     *
     * Replaces the contents of this instance with those from another instance.
     *
     * @param other Instance to copy from.
     * @return Reference to this instance.
     */
    ResponseContent &operator=(const ResponseContent &other);

    /**
     * @brief Retrieves the stored response body.
     *
     * @return Constant reference to the body string.
     */
    const std::string &getBody() const;

    /**
     * @brief Retrieves the stored MIME type.
     *
     * @return Constant reference to the MIME type string.
     */
    const std::string &getType() const;

    /**
     * @brief Retrieves the length of the body content in bytes.
     *
     * @return Size of the body string.
     */
    size_t getContentLength() const;

    /**
     * @brief Destructor.
     */
    ~ResponseContent();

private:
    std::string body_; /**< The content of the response body. */
    std::string type_; /**< The MIME type of the response content. */

    /**
     * @brief Determines the MIME type of a file based on its extension.
     *
     * Reads the list of MIME types from the file defined by @ref MIME_TYPES_PATH.
     * The expected format for each line in the MIME types file is:
     *   <mime-type> <ext1> <ext2> ... <extN>
     * For example:
     *   text/html html htm
     *   image/png png
     *
     * The function matches the extension of the given path to find
     * the corresponding type. If no match is found, sets the type to
     * "text/plain".
     *
     * @param path Path to the file whose MIME type should be determined.
     * @param mime Pointer to mime tipes object created from @ref MIME_TYPES_PATH file
     */
    void setFileType(const std::string &path, MimeTypes *mime);
};

} // namespace http