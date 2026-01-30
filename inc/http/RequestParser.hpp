#include "common/filesystem.hpp"
#include "http/HttpStatus.hpp"
#include "http/Request.hpp"
#include "http/internal/ChunkedBodyParser.hpp"
#include <ostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace http {

class RequestParser {
public:
    enum State {
        READING_HEADERS, // Waiting for '\r\n\r\n'
        HEADERS_READY,   // **PAUSE STATE**. Headers are parsed. Waiting for routing.
        READING_BODY,    // Consuming the request body.
        REQUEST_READY,   // Successfully parsed one full request.
        ERROR            // A protocol or policy error occurred.
    };

    /**
     * @param request The Request object context to populate.
     * @param maxHeaderSize Hard limit on header block size (prevents OOM).
     */
    RequestParser(Request &req, size_t maxHeaderSize);

    /**
     * @brief Resets the parser *and* the associated Request object for reuse.
     */
    void reset();

    /**
     * @brief Feeds a chunk of raw socket data into the parser's state machine.
     * @param chunk Pointer to the data buffer.
     * @param size Number of bytes in the buffer.
     * @return The new state of the parser after consuming the chunk.
     */
    State feed(char const *chunk, size_t size);

    /**
     * @brief Resumes parsing after the HEADERS_READY pause.
     *
     * This tells the parser to check the body-related headers
     * (Content-Length, Transfer-Encoding) and, crucially, the *policy*
     * from the (now-routed) Request object to prepare for body reading.
     *
     * @pre Call this *only* when state is HEADERS_READY.
     * @return The new state (e.g., READING_BODY or REQUEST_READY if no body).
     */
    State proceedReadingBody();

    State state() const;
    HttpStatus errorStatus() const;

private:
    /**
     * @brief Internal: Tries to find and parse the full header block.
     * @return New state (READING_HEADERS or HEADERS_READY).
     */
    State parseHeaders();

    /**
     * @brief Internal: Tries to consume data from the buffer for the body.
     * @return New state (READING_BODY or REQUEST_READY).
     */
    State parseBody();

    void handleChunkedBody();
    void handleContentLengthBody();
    bool writeToBodyFile(const std::string &data);

    /**
     * @brief Sets the parser to an error state and updates the Request's status.
     */
    State setError(HttpStatus status);

    void setRequestReady();

    State state_;
    HttpStatus errorStatus_;

    std::string buffer_;

    size_t maxHeaderSize_;  // Hard-coded server limit
    size_t maxContentSize_; // Policy-driven limit, set in proceedReadingBody()

    size_t contentLength_;
    size_t bytesWrittenToBody_;
    bool isContentChunked_;

    Request &request_;

    ChunkedBodyParser chunkParser_;
};

std::ostream &operator<<(std::ostream &o, RequestParser::State st);

}; // namespace http
