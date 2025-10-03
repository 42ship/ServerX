#pragma once

#include "IArgument.hpp"
#include <vector>

namespace config {

class ConcatenatedValue : public IArgument {
public:
    ~ConcatenatedValue();
    ConcatenatedValue(const ConcatenatedValue &other);
    ConcatenatedValue &operator=(const ConcatenatedValue &other);

    void add(IArgument *arg);
    std::string evaluate(http::HttpRequest const &ctx) const;
    ArgumentType getType() const { return ARG_CONCATENATED; }
    std::string getRawValue() const;
    IArgument *clone() const;

private:
    std::vector<ArgumentPtr> variables_;
};

} // namespace config
