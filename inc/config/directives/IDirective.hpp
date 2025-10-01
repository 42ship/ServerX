#pragma once

#include "config/internal/Block.hpp"
#include <string>

namespace config {

class IDirective {
public:
    virtual ~IDirective() {}
    virtual void process(Block &block, StringVector const &args) const = 0;
    virtual std::string const &getName() const = 0;
};

} // namespace config
