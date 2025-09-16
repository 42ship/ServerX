#include "config/pipeline/DirectiveHandler.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Block.hpp"
#include "config/internal/utils.hpp"
#include "config/ServerBlock.hpp"
#include "config/LocationBlock.hpp"
#include <string>

namespace {
bool parseListen(const std::string &s, std::string &addr_part, std::string &port_part);
}

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
    std::string port_str;
    if (!parseListen(args[0], b.address_, port_str)) {
        throw ConfigError("Listen directive '" + args[0] + "' has an invalid format.");
    }
    if (!utils::isValidPort(port_str)) {
        throw ConfigError("Port '" + port_str + "' is not a valid.");
    } else
        b.port_ = utils::fromString<int>(port_str);
    b.directives_["listen"] = args;
}

void DirectiveHandler::handleRoot(Block &b, StringVector const &args) {
    if (args.size() != 1)
        throw ConfigError("'root' directive requires exactly one argument.");
    b.directives_["root"] = args;
}

} // namespace config

namespace {

bool parseListen(const std::string &s, std::string &addr_part, std::string &port_part) {
    if (s.empty())
        return false;

    size_t cpos = s.find_last_of(':');

    if (cpos == std::string::npos) {
        bool is_numeric = (s.find_first_not_of("0123456789") == std::string::npos);
        if (is_numeric) {
            port_part = s;
        } else {
            addr_part = s;
        }
    } else {
        addr_part = s.substr(0, cpos);
        port_part = s.substr(cpos + 1);

        if (addr_part.empty() || port_part.empty())
            return false;
    }
    return true;
}

} // namespace
