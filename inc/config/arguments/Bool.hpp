#pragma once

#include "config/arguments/IArgument.hpp"

namespace config {

class Bool : public IArgument {
public:
    Bool(bool value);
    Bool(std::string const &value);
    Bool(const char *value);
    virtual ~Bool();

    virtual std::string evaluate(http::Request const &) const;
    virtual ArgumentType getType() const;
    virtual std::string getRawValue() const;
    virtual IArgument *clone() const;

    bool getValue() const;

private:
    void init(std::string const &value);
    bool value_;
};

} // namespace config
