#pragma once

#include "IArgument.hpp"

namespace config {

class String : public IArgument {
public:
    String(std::string const &s) : value_(s) {}
    void setValue(const std::string &value) { value_ = value; }
    std::string evaluate(http::Request const &) const { return value_; }
    ArgumentType getType() const { return ARG_STRING; }
    std::string getRawValue() const { return value_; }
    IArgument *clone() const { return new String(*this); }

private:
    std::string value_;
};

} // namespace config
