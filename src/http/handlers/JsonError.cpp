#include "http/Handler.hpp"
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace http {

struct Key {
    HttpStatus code;
    std::string message;
    bool operator<(Key const &o) const {
        return (code < o.code) || (code == o.code && message < o.message);
    }
};

typedef std::map<Key, std::string> JsonErrorPageCache;

static std::string createJsonErrorBody(HttpStatus code, char const *message) {
    std::ostringstream body;
    body << "{ \"" << code << "\": \"" << message << "\" }";
    return body.str();
}

static const std::string &getCachedJsonErrorBody(HttpStatus code, const char *message) {
    static JsonErrorPageCache cache;
    const std::string m = message ? message : "";
    Key k;
    k.code = code;
    k.message = m;

    JsonErrorPageCache::const_iterator it = cache.find(k);
    if (it != cache.end())
        return it->second;

    return cache.insert(std::make_pair(k, createJsonErrorBody(code, m.c_str()))).first->second;
}

void JsonErrorHandler::populateResponse(Response &response) {
    std::string const &body =
        getCacherJsonErrorBody(response.status(), response.reasonPhrase().c_str());
    response.setBodyInMemory(body, "application/json");
}

} // namespace http
