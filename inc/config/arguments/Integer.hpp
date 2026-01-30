#pragma once

#include "IArgument.hpp"

namespace config {

class Integer : public IArgument {
public:
    explicit Integer(size_t val);
    std::string evaluate(http::Request const &) const;
    size_t getIntValue() const;
    ArgumentType getType() const { return ARG_INTEGER; }
    std::string getRawValue() const { return sValue_; }
    IArgument *clone() const { return new Integer(*this); }

private:
    size_t iValue_;
    std::string sValue_;
};

} // namespace config
