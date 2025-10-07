#pragma once

#include "config/internal/Block.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

class IDirective {
public:
    virtual ~IDirective() {}
    virtual void process(Block &block, ParsedDirectiveArgs const &args) const = 0;
    virtual std::string const &getName() const = 0;
};

} // namespace config
