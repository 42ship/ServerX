#pragma once

#include "http/Request.hpp"
#include <string>

namespace config {
class ServerBlock;
class LocationBlock;
} // namespace config

/**
 * @class TestableRequest
 * @brief A wrapper around http::Request that exposes protected members for testing.
 */
class TestableRequest : public http::Request {
public:
    TestableRequest() {}
    TestableRequest(const config::ServerBlock *s, const config::LocationBlock *l) {
        this->server(s);
        this->location(l);
    }

    TestableRequest &set(http::RequestStartLine::Method m, const std::string &p) {
        this->method(m);
        this->path(p);
        return *this;
    }

    // Expose protected setters as public for easier testing
    using http::Request::body;
    using http::Request::location;
    using http::Request::method;
    using http::Request::path;
    using http::Request::remoteAddr;
    using http::Request::server;
    using http::Request::version;

    // Expose getters (which may be inherited as public anyway, but explicit is better)
    using http::Request::bodyPath;
};
