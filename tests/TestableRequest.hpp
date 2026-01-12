#include "http/Request.hpp"

class TestableRequest : public http::Request {
public:
    using Request::location;
    using Request::method;
    using Request::path;
    using Request::remoteAddr;
    using Request::server;
    using Request::version;
};
