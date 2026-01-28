#pragma once

#include "http/Handler.hpp"
#include "http/MimeTypes.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

namespace http {

class DirectoryListingHandler : public IHandler {
public:
    static void handle(Request const &req, Response &res, MimeTypes const &mime);
};

} // namespace http
