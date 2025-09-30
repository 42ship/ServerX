#pragma once

#include "../LocationBlock.hpp"
#include "../ServerBlock.hpp"
#include "../internal/Block.hpp"

namespace config {

class DirectiveHandler {
public:
    static DirectiveHandler &getInstance();

    void process(ServerBlock &block, const std::string &key, const StringVector &args);
    void process(ServerBlock &block, DirectiveMap const &map);
    void process(LocationBlock &block, const std::string &key, const StringVector &args);
    void process(LocationBlock &block, DirectiveMap const &map);

private:
    void handleListen(ServerBlock &server, const StringVector &args);
    void handleRoot(Block &block, const StringVector &args);

    typedef void (DirectiveHandler::*GenericHandler)(Block &, const StringVector &);
    typedef void (DirectiveHandler::*ServerHandler)(ServerBlock &, const StringVector &);
    typedef void (DirectiveHandler::*LocationHandler)(LocationBlock &, const StringVector &);

    typedef std::map<std::string, GenericHandler> GenericHandlerMap;
    typedef std::map<std::string, ServerHandler> ServerHandlerMap;
    typedef std::map<std::string, LocationHandler> LocationHandlerMap;

    GenericHandlerMap genericHandlers_;
    ServerHandlerMap serverHandlers_;
    LocationHandlerMap locationHandlers_;

    DirectiveHandler();
    ~DirectiveHandler();
    DirectiveHandler(const DirectiveHandler &);
    DirectiveHandler &operator=(const DirectiveHandler &);
};

} // namespace config
