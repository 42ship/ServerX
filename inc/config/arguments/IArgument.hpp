#pragma once

#include <string>

namespace http {
class HttpRequest;
}

namespace config {

enum ArgumentType { ARG_STRING, ARG_INTEGER, ARG_VARIABLE, ARG_CONCATENATED };

class IArgument {
public:
    virtual ~IArgument() {}
    virtual std::string evaluate(http::HttpRequest const &) const = 0;
    virtual ArgumentType getType() const = 0;
    virtual std::string getRawValue() const = 0;
    virtual IArgument *clone() const = 0;
};

char const *getArgumentTypeName(ArgumentType);

typedef IArgument *ArgumentPtr;

} // namespace config
