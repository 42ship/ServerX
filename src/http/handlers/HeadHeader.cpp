#include "http/Handler.hpp"

#include "common/string.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "utils/Logger.hpp"

namespace http {

void HeadHeaderHandler::handle(Request const &req, Response &res, MimeTypes const &mimeTypes) {
    StaticFileHandler::handle(req, res, mimeTypes);

    if (res.status() != OK) {
        res.setNoBody(false);
        return;
    }
	LOG_DEBUG("HeadHeaderHandler::handle: removing body for HEAD request");
    res.setNoBody(false);
}

} // namespace http