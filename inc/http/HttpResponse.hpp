#pragma once

#include "http/Headers.hpp"
#include "http/ResponseBody.hpp"
#include "http/ResponseStatus.hpp"

namespace http {

class ResponseStartLine {
public:
    std::string protocol;
    ResponseStatus statusCode;
    std::string reasonPhrase;
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
    ~HttpResponse();

    ResponseStartLine startLine;
    Headers headers;
    IResponseBody *body;

    void setNoBody();
    void setBodyInMemory(std::string const &data, std::string const &contentType);
    void setBodyFromFile(std::string const &fpath, std::string const &contentType);
    void setBodyFromCgi(int pipe_fd, std::string const &firstChunk);

private:
    HttpResponse(HttpResponse const &);
    HttpResponse const &operator=(HttpResponse const &);
};

} // namespace http
