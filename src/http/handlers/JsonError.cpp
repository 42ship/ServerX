#include "http/Handler.hpp"
#include <map>
#include <sstream>
#include <string>
#include <utility>

namespace http {

#if 0
typedef map<Key, vector<char> > JsonErrorPageCache;

static std::string createJsonErrorBody(HttpStatus code, char const *message) {
    std::ostringstream body;
    body << "{ \"" << code << "\": \"" << message << "\" }";
    return body.str();
}

// typedef std::map<Key, std::vector<char> > JsonErrorPageCache;

static const std::vector<char> &getCacherJsonErrorBody(Status code, const std::string &message) {
    static JsonErrorPageCache cache;
    const std::string m = !message.empty() ? message : "";
    Key k;
    k.code = code;
    k.message = m;

    JsonErrorPageCache::const_iterator it = cache.find(k);
    if (it != cache.end())
        return it->second;

    return cache.insert(std::make_pair(k, createJsonErrorBody(code, m.c_str()))).first->second;
}
#endif

void JsonErrorHandler::populateResponse(Response const &response) { (void)response; }

} // namespace http
