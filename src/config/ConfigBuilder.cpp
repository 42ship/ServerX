#include "config/ConfigBuilder.hpp"
#include "config/utils.hpp"
#include "config/ConfigException.hpp"
#include <stdexcept>

namespace config {

ConfigBuilder::ServerHandlerMap const &ConfigBuilder::getServerDirectiveHandlers() {
    static ServerHandlerMap map;
    if (!map.empty())
        return map;
    map["listen"] = &ConfigBuilder::handleListen;
    map["server_name"] = &ConfigBuilder::handleServerName;
    map["root"] = &ConfigBuilder::handleRoot;
    return map;
}

ConfigBuilder::LocationHandlerMap const &ConfigBuilder::getLocationDirectiveHandlers() {
    static LocationHandlerMap map;
    if (!map.empty())
        return map;
    map["root"] = &ConfigBuilder::handleRoot;
    map["index"] = &ConfigBuilder::handleIndex;
    return map;
}

ServerBlockVec ConfigBuilder::build(ConfigNodeVec const &nodes) {
    if (nodes.empty())
        throw ConfigError("Config file is empty");

    ConfigBuilder cb;
    cb.servers_.reserve(nodes.size());
    for (size_t i = 0; i < nodes.size(); i++) {
        cb.buildServer(nodes[i]);
    }
    return cb.servers_;
}

void ConfigBuilder::buildServer(ConfigNode const &node) {
    if (node.name != "server")
        throw std::runtime_error("ConfigBuilder unexpected call of buildServer non matching name");
    ServerBlock cf;
    buildServerDirectives(cf, node.directives);
    buildServerChildren(cf, node.children);
    servers_.push_back(cf);
}

void ConfigBuilder::buildServerDirectives(ServerBlock &conf, DirectiveMap const &directives) {
    ServerHandlerMap handlers = getServerDirectiveHandlers();
    for (DirectiveMap::const_iterator it = directives.begin(); it != directives.end(); ++it) {
        ServerHandlerMap::const_iterator handler;
        std::string const &name = it->first;
        handler = handlers.find(name);
        if (handler == handlers.end())
            throw ConfigError("Unknown directive '" + name + "' in server block");
        (this->*handler->second)(conf, it->second);
    }
}

void ConfigBuilder::buildServerChildren(ServerBlock &conf, ConfigNodeVec const &nodes) {
    typedef void (ConfigBuilder::*NodeHandler)(ServerBlock &, ConfigNode const &);
    typedef std::map<std::string, NodeHandler> NodeHandlerMap;

    NodeHandlerMap handlers;
    handlers["location"] = &ConfigBuilder::buildLocation;

    for (ConfigNodeVec::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        NodeHandlerMap::const_iterator handler = handlers.find(it->name);
        if (handler == handlers.end()) {
            throw ConfigError("Unknown block '" + it->name + "' in server block'");
        }
        (this->*handler->second)(conf, *it);
    }
}

void ConfigBuilder::buildLocation(ServerBlock &conf, ConfigNode const &node) {
    if (node.args.size() != 1) {
        throw ConfigError("Location block requires exactly 1 path argument");
    }
    LocationBlock loc;
    loc.path = node.args[0];
    if (conf.locations_.count(loc.path) > 0) {
        issue_warning("Duplicate location '" + loc.path +
                      "' in location block. Ignoring subsequent directives.");
        return;
    }

    LocationHandlerMap handlers = getLocationDirectiveHandlers();
    DirectiveMap const &directives = node.directives;
    for (DirectiveMap::const_iterator it = directives.begin(); it != directives.end(); ++it) {
        LocationHandlerMap::const_iterator handler;
        handler = handlers.find(it->first);
        if (handler == handlers.end()) {
            throw ConfigError("Unknown directive '" + it->first + "' in server block");
        }
        (this->*handler->second)(loc, it->second);
    }
    conf.locations_[loc.path] = loc;
}

void ConfigBuilder::handleListen(ServerBlock &cfg, DirectiveArgs const &args) {
    if (args.size() == 0) {
        issue_warning("No port specified for the 'listen' directive. Using default port.");
        cfg.setDefaultPort();
        return;
    }
    if (args.size() > 1) {
        issue_warning(
            "Multiple ports provided for the 'listen' directive. Only the first port specified (" +
            args[0] + ") will be used.");
    }
    utils::IpInfo info;
    if (!utils::extractIpInfo(args[0], info)) {
        throw ConfigError("Listen directive '" + args[0] + "' is invalid");
    }
    if (info.port == -1) {
        cfg.setDefaultPort();
    } else {
        cfg.port_ = info.port;
    }
    if (info.ip.empty()) {
        cfg.setDefaultAddress();
    } else {
        cfg.address_ = info.ip;
    }
}

void ConfigBuilder::handleServerName(ServerBlock &cfg, DirectiveArgs const &args) {
    cfg.serverNames_.reserve(cfg.serverNames_.size() + args.size());
    for (DirectiveArgs::const_iterator it = args.begin(); it != args.end(); ++it) {
        cfg.serverNames_.push_back(*it);
    }
}

void ConfigBuilder::handleRoot(ServerBlock &cfg, DirectiveArgs const &args) {
    if (args.size() != 1) {
        throw ConfigError("Root directive requires exactly 1 argument");
    }
    cfg.root = args[0];
}

void ConfigBuilder::handleRoot(LocationBlock &loc, DirectiveArgs const &args) {
    if (args.size() != 1) {
        throw ConfigError("Root directive requires exactly 1 argument");
    }
    loc.root = args[0];
}

void ConfigBuilder::handleIndex(LocationBlock &loc, DirectiveArgs const &args) {
    loc.index.reserve(args.size());
    for (DirectiveArgs::const_iterator it = args.begin(); it != args.end(); ++it) {
        loc.index.push_back(*it);
    }
}

} // namespace config
