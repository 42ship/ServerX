#include "config/pipeline/DirectiveHandler.hpp"
#include "config/ConfigException.hpp"
#include "config/LocationBlock.hpp"
#include "config/ServerBlock.hpp"
#include "config/internal/Block.hpp"
#include <string>

namespace config {

DirectiveHandler::DirectiveHandler() {
    genericHandlers_["root"] = &DirectiveHandler::handleRoot;

    serverHandlers_["listen"] = &DirectiveHandler::handleListen;

    // locationHandlers_["blahblah"] = &DirectiveHandler::blahblah;
}

DirectiveHandler::~DirectiveHandler() {
}

DirectiveHandler &DirectiveHandler::getInstance() {
    static DirectiveHandler handler;
    return handler;
}

void DirectiveHandler::process(ServerBlock &b, DirectiveMap const &map) {
    for (DirectiveMap::const_iterator it = map.begin(); it != map.end(); ++it) {
        process(b, it->first, it->second);
    }
}

void DirectiveHandler::process(LocationBlock &b, DirectiveMap const &map) {
    for (DirectiveMap::const_iterator it = map.begin(); it != map.end(); ++it) {
        process(b, it->first, it->second);
    }
}

void DirectiveHandler::process(ServerBlock &b, std::string const &key, StringVector const &args) {
    ServerHandlerMap::const_iterator server_it = serverHandlers_.find(key);
    if (server_it != serverHandlers_.end()) {
        (this->*(server_it->second))(b, args);
        return;
    }

    GenericHandlerMap::const_iterator generic_it = genericHandlers_.find(key);
    if (generic_it != genericHandlers_.end()) {
        (this->*(generic_it->second))(b, args);
        return;
    }

    b.directives_[key] = args;
}

void DirectiveHandler::process(LocationBlock &b, std::string const &key, StringVector const &args) {
    LocationHandlerMap::const_iterator location_it = locationHandlers_.find(key);
    if (location_it != locationHandlers_.end()) {
        (this->*(location_it->second))(b, args);
        return;
    }

    GenericHandlerMap::const_iterator generic_it = genericHandlers_.find(key);
    if (generic_it != genericHandlers_.end()) {
        (this->*(generic_it->second))(b, args);
        return;
    }

    b.directives_[key] = args;
}

void DirectiveHandler::handleListen(ServerBlock &b, StringVector const &args) {
    if (args.size() != 1) {
        throw ConfigError("'listen' directive requires exactly one argument.");
    }
    b.setListen(args[0]);
    b.directives_["listen"] = args;
}

void DirectiveHandler::handleRoot(Block &b, StringVector const &args) {
    if (args.size() != 1)
        throw ConfigError("'root' directive requires exactly one argument.");
    b.directives_["root"] = args;
}

} // namespace config
