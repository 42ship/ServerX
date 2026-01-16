#include "config/pipeline/DirectiveHandler.hpp"
#include "config/directives/ErrorPageDirective.hpp"
#include "config/directives/IDirective.hpp"
#include "config/directives/ListenDirective.hpp"
#include "config/directives/MaxBodySize.hpp"
#include "config/directives/ReturnDirective.hpp"
#include "config/directives/RootDirective.hpp"
#include "config/directives/UploadPathDirective.hpp"
#include <vector>

namespace config {

static IDirective **getBuiltInDirectives(size_t &count) {
    static IDirective *directives[] = {new ListenDirective(),     new RootDirective(),
                                       new ErrorPageDirective(),  new ReturnDirective(),
                                       new UploadPathDirective(), new MaxBodySize()};
    count = sizeof(directives) / sizeof(directives[0]);
    return directives;
}

DirectiveHandler::DirectiveHandler() {
    size_t count;
    IDirective **directives = getBuiltInDirectives(count);
    for (size_t i = 0; i < count; i++)
        registerHandler(directives[i]);
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
