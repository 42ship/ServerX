#pragma once

#include "IArgument.hpp"

namespace config {

class Variable : public IArgument {
public:
    Variable(std::string const &s) : varName_(s) {}
    std::string evaluate(http::Request const &) const;
    ArgumentType getType() const { return ARG_VARIABLE; }
    std::string getRawValue() const { return varName_; }
    IArgument *clone() const { return new Variable(*this); }

private:
    std::string varName_;
};

} // namespace config
