#pragma once

#include "config/directives/IDirective.hpp"
#include <string>

namespace config {

class AutoIndexDirective : public IDirective {
public:
    virtual ~AutoIndexDirective() {}
    virtual void process(Block &block, ParsedDirectiveArgs const &args) const;
    virtual std::string const &getName() const { return name_; }

private:
    static const std::string name_;
};

} // namespace config
