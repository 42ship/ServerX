#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"

namespace http {

HttpResponse NotFoundHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                     config::LocationBlock const *l) const {
    (void)l;
    (void)s;
    return error_pages::generateErrorResponse(NOT_FOUND, req.version);
}
} // namespace http
