#pragma once

#include "IArgument.hpp"

namespace config {

class String : public IArgument {
public:
    std::string evaluate(http::HttpRequest const &) const { return value_; }

private:
    std::string value_;
};

} // namespace config
