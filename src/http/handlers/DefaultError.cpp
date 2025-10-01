#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"
#include "http/error_pages.hpp"

using namespace std;

namespace http {

HttpResponse DefaultErrorHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                         config::LocationBlock const *l) const {
    (void)s;
    (void)l;
    if (req.method == utils::POST) {
        return error_pages::generateJsonErrorResponse(req.status, req.version);
    }
    return error_pages::generateErrorResponse(req.status, req.version);
}

} // namespace http
