#pragma once

#include "config/ServerBlock.hpp"
#include "config/LocationBlock.hpp"

namespace http {

class IHandler;

/**
 * @struct RouterResult
 * @brief A data container for the result of a routing decision.
 *
 * This struct packages the selected handler and its relevant configuration
 * context to be used by the main server loop.
 */
struct RouterResult {
    RouterResult(IHandler const &h, config::ServerBlock const *s = NULL,
                 config::LocationBlock const *l = NULL)
        : handler(h), server(s), location(l) {
    }

    IHandler const &handler;               //!< The selected handler for the request.
    config::ServerBlock const *server;     //!< The matched server block context.
    config::LocationBlock const *location; //!< The matched location block context.
};

} // namespace http
