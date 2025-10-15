#pragma once

#include "config/internal/types.hpp"

namespace config {

class ArgumentFactory {
public:
    static ArgumentPtr get(Token const &);
    static ArgumentVector get(ParsedDirectiveArgs const &);

private:
    static ArgumentPtr handleStringVariable(std::string const &);
};

} // namespace config
