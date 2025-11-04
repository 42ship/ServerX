#include "config/arguments/Variable.hpp"
#include "http/Request.hpp"

namespace config {

typedef std::string (*FuncVar)(http::Request const &req);
typedef std::map<std::string, FuncVar> FuncMap;

FuncMap const &getMap();

std::string Variable::evaluate(http::Request const &req) const {
    FuncMap const &map = getMap();
    FuncMap::const_iterator it = map.find(varName_);
    if (it == map.end())
        return "";
    return it->second(req);
}

std::string HostFunc(http::Request const &req) { return req.headers().get("Host"); }

FuncMap const &getMap() {
    static FuncMap map;

    if (!map.empty())
        return map;
    map["host"] = HostFunc;
    return map;
}

} // namespace config
