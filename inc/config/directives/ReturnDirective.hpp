#pragma once

#include "IDirective.hpp"

namespace config {

class ReturnDirective : public IDirective {
public:
    void process(Block &block, StringVector const &args) const;
    std::string const &getName() const { return name_; }

private:
    static const std::string name_;
};

} // namespace config
