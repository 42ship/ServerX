#pragma once

#include "IArgument.hpp"

namespace config {

class Integer : public IArgument {
public:
    Integer(int val);
    std::string evaluate(http::HttpRequest const &) const;
    int getIntValue() const;
    ArgumentType getType() const { return ARG_INTEGER; }
    std::string getRawValue() const { return sValue_; }
    IArgument *clone() const { return new Integer(*this); }

private:
    int iValue_;
    std::string sValue_;
};

} // namespace config
