
#include "HttpRequest.hpp"
#include "common/filesystem.hpp"
#include "http/Headers.hpp"
#include "http/RequestLine.hpp"
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace http {

class RequestParser {
public:
    enum RequestState { READING_HEADERS, HEADERS_READY, READING_BODY, REQUEST_READY, ERROR };

    RequestParser(size_t maxContentSize);

    void reset();
    RequestState getState() const;
    size_t getContentLength() const;
    RequestLine const &getStartLine() const;
    http::Headers const &getHeaders() const;
    RequestParser &proceedReadingBody();
    RequestParser &setMaxContentSize(size_t size);
    HttpRequest &getRequestContext();

    RequestState addIncomingChunk(char const *chunk, size_t size);

private:
    void readHeaders();
    // TODO: when router sets the routing we can check outside of this class if the body is allowed
    void handleBody();

    void handleChunkedBody();
    void handleContentLengthBody();

    std::string buffer_;

    // Properties
    size_t maxContentSize_;

    // State of the request
    RequestState state_;
    bool isContentChunked_;

    size_t contentLength_;
    size_t bytesWrittenToBody_;

    // Final objects
    HttpRequest reqContext_;
    RequestLine &startLine_;
    http::Headers &headers_;
    utils::TempFile bodyFile_;
    // Final objects
};

}; // namespace http
