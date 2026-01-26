#include "config/pipeline/DirectiveHandler.hpp"
#include "config/directives/AliasDirective.hpp"
#include "config/directives/AllowMethodsDirective.hpp"
#include "config/directives/CgiPassDirective.hpp"
#include "config/directives/ClientMaxBodySize.hpp"
#include "config/directives/ErrorPageDirective.hpp"
#include "config/directives/IDirective.hpp"
#include "config/directives/IndexDirective.hpp"
#include "config/directives/ListenDirective.hpp"
#include "config/directives/ReturnDirective.hpp"
#include "config/directives/RootDirective.hpp"
#include "config/directives/ServerNameDirective.hpp"
#include "config/directives/UploadPathDirective.hpp"

namespace config {

DirectiveHandler::DirectiveHandler() {
    registerHandler(new ListenDirective);
    registerHandler(new RootDirective);
    registerHandler(new ErrorPageDirective);
    registerHandler(new ReturnDirective);
    registerHandler(new ClientMaxBodySize);
    registerHandler(new IndexDirective);
    registerHandler(new AliasDirective);
    registerHandler(new ServerNameDirective);
    registerHandler(new UploadPathDirective);
    registerHandler(new AllowMethodsDirective);
    registerHandler(new CgiPassDirective);
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
