#pragma once

#include "IDirective.hpp"
#include "config/internal/Block.hpp"

namespace config {

class ListenDirective : public IDirective {
public:
    void process(Block &block, ArgumentVector const &args) const;
    std::string const &getName() const { return name_; }

private:
    static const std::string name_;
};

} // namespace config
