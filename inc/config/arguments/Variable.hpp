#pragma once

#include "IArgument.hpp"

namespace config {

class Variable : public IArgument {
public:
    std::string evaluate(http::HttpRequest const &) const;

private:
    std::string varName_;
};

} // namespace config
