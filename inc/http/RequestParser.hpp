#include "common/filesystem.hpp"
#include "http/Headers.hpp"
#include "http/HttpStatus.hpp"
#include "http/Request.hpp"
#include <string>
#include <sys/types.h>
#include <unistd.h>

namespace http {

class RequestParser {
public:
    enum RequestState { READING_HEADERS, HEADERS_READY, READING_BODY, REQUEST_READY, ERROR };

    RequestParser(Request &req, size_t maxContentSize, size_t maxHeaderSize_);

    void reset();
    RequestState getState() const;
    size_t getContentLength() const;
    RequestStartLine const &getStartLine() const;
    Headers const &getHeaders() const;
    RequestState proceedReadingBody();
    RequestParser &setMaxContentSize(size_t size);

    RequestState addIncomingChunk(char const *chunk, size_t size);

private:
    void readHeaders();
    // TODO: when router sets the routing we can check outside of this class if the body is allowed
    void handleBody();

    void handleChunkedBody();
    void handleContentLengthBody();

    void setError(HttpStatus);

    std::string buffer_;

    // Properties
    size_t maxContentSize_;
    size_t maxHeaderSize_;

    // State of the request
    RequestState state_;
    bool isContentChunked_;

    size_t contentLength_;
    size_t bytesWrittenToBody_;

    // Final objects
    Request &request_;
    RequestStartLine &startLine_;
    Headers &headers_;
    utils::TempFile bodyFile_;
    // Final objects
};

}; // namespace http
