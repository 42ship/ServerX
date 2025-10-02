#pragma once

#include <string>

namespace http {
class HttpRequest;
}

namespace config {

class IArgument {
public:
    virtual ~IArgument() {}
    virtual std::string evaluate(http::HttpRequest const &) const = 0;
};

typedef IArgument *ArgumentPtr;

} // namespace config
