#pragma once

#include "config/Block.hpp"
#include "config/directives/IDirective.hpp"
#include "config/internal/Token.hpp"

namespace config {

class IDirective;

class DirectiveHandler {
public:
    static DirectiveHandler &getInstance();
    ~DirectiveHandler();

    template <typename T>
    void process(T &block, std::string const &key, ParsedDirectiveArgs const &args) {
        HandlerMap::const_iterator it = handlers_.find(key);
        if (it != handlers_.end()) {
            it->second->process(block, args);
        } else {
            block.add(key, args);
        }
    }

    template <typename T> void process(T &block, ParsedDirectiveMap const &map) {
        for (ParsedDirectiveMap::const_iterator it = map.begin(); it != map.end(); ++it) {
            process(block, it->first, it->second);
        }
    }

private:
    typedef std::map<std::string, IDirective *> HandlerMap;
    HandlerMap handlers_;
    void registerHandler(IDirective *h);

    DirectiveHandler();
    DirectiveHandler(const DirectiveHandler &);
    DirectiveHandler &operator=(const DirectiveHandler &);
};

} // namespace config
