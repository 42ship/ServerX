#pragma once

#include "IArgument.hpp"

namespace config {

class Integer : public IArgument {
public:
    Integer(int val);
    std::string evaluate(http::HttpRequest const &) const;
    int getIntValue() const;

private:
    int iValue_;
    std::string sValue_;
};

} // namespace config
