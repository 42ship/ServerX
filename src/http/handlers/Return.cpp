#include "config/LocationBlock.hpp"
#include "config/arguments/IArgument.hpp"
#include "config/arguments/Integer.hpp"
#include "config/internal/types.hpp"
#include "http/Handler.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"

namespace http {

void http::ReturnHandler::handle(Request const &req, Response &res) {
    CHECK_FOR_SERVER_AND_LOCATION(req, res);
    if (!req.location()->has("return")) {
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    config::ArgumentVector args = req.location()->get("return");

    if (args.empty()) {
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }

    if (args.size() == 1) {
        return (void)res.status(FOUND).headers().add(
            "location", req.location()->getFirstEvaluatedString("return", req));
    }

    if (args[0]->getType() != config::ARG_INTEGER) {
        return (void)res.status(INTERNAL_SERVER_ERROR);
    }
    int num = static_cast<config::Integer const *>(args[0])->getIntValue();
    HttpStatus status = toHttpStatus(num);
    std::string const &secondArgument = args[1]->evaluate(req);
    if (num < 300 || num >= 400) {
        return (void)res.status(status).setBodyInMemory(secondArgument, "text/plain");
    }
    return (void)res.status(status).headers().add("location", secondArgument);
}

} // namespace http
