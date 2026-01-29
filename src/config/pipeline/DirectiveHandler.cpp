#include "config/directives/AutoIndexDirective.hpp"
#include "config/directives/ErrorPageDirective.hpp"
#include "config/directives/IDirective.hpp"
#include "config/directives/ListenDirective.hpp"
#include "config/directives/MaxBodySize.hpp"
#include "config/directives/ReturnDirective.hpp"
#include "config/directives/RootDirective.hpp"
#include "config/directives/UploadPathDirective.hpp"
#include "config/pipeline/DirectiveHandler.hpp"

namespace config {

DirectiveHandler::DirectiveHandler() {
    registerHandler(new ListenDirective);
    registerHandler(new RootDirective);
    registerHandler(new ErrorPageDirective);
    registerHandler(new ReturnDirective);
    registerHandler(new MaxBodySize);
    registerHandler(new UploadPathDirective);
    registerHandler(new AutoIndexDirective);
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
