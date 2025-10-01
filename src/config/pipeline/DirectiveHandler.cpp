#include "config/pipeline/DirectiveHandler.hpp"
#include "config/directives/IDirective.hpp"
#include "config/directives/ListenDirective.hpp"
#include "config/directives/ReturnDirective.hpp"
#include "config/directives/RootDirective.hpp"

namespace config {

DirectiveHandler::DirectiveHandler() {
    registerHandler(new ListenDirective);
    registerHandler(new RootDirective);
    registerHandler(new ReturnDirective);
}

void DirectiveHandler::registerHandler(IDirective *h) {
    if (!h)
        return;
    handlers_[h->getName()] = h;
}

DirectiveHandler::~DirectiveHandler() {
    for (HandlerMap::iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
        delete it->second;
    }
}

DirectiveHandler &DirectiveHandler::getInstance() {
    static DirectiveHandler handler;
    return handler;
}

} // namespace config
