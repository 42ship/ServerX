#include "config/arguments/ArgumentFactory.hpp"
#include "common/string.hpp"
#include "config/arguments/ConcatenatedValue.hpp"
#include "config/arguments/Integer.hpp"
#include "config/arguments/String.hpp"
#include "config/arguments/Variable.hpp"
#include "config/internal/ConfigException.hpp"
#include "config/internal/Token.hpp"
#include "config/internal/types.hpp"
#include <string>

namespace config {

ArgumentPtr ArgumentFactory::get(Token const &t) {
    switch (t.type) {
    case STRING:
        return handleStringVariable(t.literal);
    case NUMBER:
        return new Integer(utils::fromString<int>(t.literal));
    default:
        throw ConfigError("Internal error: unimplemented token in ArgumentFactory");
    }
}

ArgumentVector ArgumentFactory::get(ParsedDirectiveArgs const &args) {
    ArgumentVector res;
    res.reserve(args.size());
    for (size_t i = 0; i < args.size(); i++) {
        res.push_back(get(args[i]));
    }
    return res;
}

bool isValidVariableChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
}

size_t getVariableLength(std::string const &s, size_t pos) {
    size_t i = pos;
    while (i < s.length() && isValidVariableChar(s[i])) {
        i++;
    }
    return i - pos;
}

ArgumentPtr ArgumentFactory::handleStringVariable(std::string const &s) {
    if (s.find('$') == std::string::npos)
        return new String(s);
    if (s.length() > 1 && s[0] == '$') {
        size_t vlength = getVariableLength(s, 1);
        if (vlength > 0 && vlength == s.length() - 1)
            return new Variable(s.substr(1));
    }

    ConcatenatedValue *var = new ConcatenatedValue;
    size_t current_pos = 0;
    while (current_pos < s.size()) {
        size_t var_start = s.find('$', current_pos);

        if (var_start == std::string::npos) {
            var->add(new String(s.substr(current_pos)));
            break;
        }
        if (var_start > current_pos) {
            var->add(new String(s.substr(current_pos, var_start - current_pos)));
        }

        size_t var_name_pos = var_start + 1;
        size_t vlength = getVariableLength(s, var_start + 1);

        if (vlength == 0) {
            delete var;
            throw ConfigError("'" + s + "': has empty or invalid variable");
        }

        var->add(new Variable(s.substr(var_name_pos, vlength)));
        current_pos = var_name_pos + vlength;
    }

    return var;
}

} // namespace config
