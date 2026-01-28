#include "config/arguments/IArgument.hpp"

namespace config {

char const *getArgumentTypeName(ArgumentType type) {
    switch (type) {
    case ARG_STRING:
        return "String";
    case ARG_INTEGER:
        return "Integer";
    case ARG_VARIABLE:
        return "Variable";
    case ARG_CONCATENATED:
        return "Concatenated";
    case ARG_BOOL:
        return "Boolean";
    default:
        return "Unknown";
    }
}

} // namespace config
