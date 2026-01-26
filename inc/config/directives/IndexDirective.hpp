#pragma once

#include "IDirective.hpp"

namespace config {

class IndexDirective : public IDirective {
public:
    void process(Block &b, ParsedDirectiveArgs const &args) const;
    std::string const &getName() const { return name_; }

private:
    static const std::string name_;
};

} // namespace config
