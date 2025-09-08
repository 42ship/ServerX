#include "http/Handler.hpp"
#include "http/HttpResponse.hpp"

namespace http {

HttpResponse CGIHandler::handle(HttpRequest const &req, config::ServerBlock const *s,
                                config::LocationBlock const *l) const {
    HttpResponse res;
    (void)req;
    (void)s;
    (void)l;
    return res;
}
} // namespace http
