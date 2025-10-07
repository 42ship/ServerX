#pragma once

#include <map>
#include <string>
#include <vector>

namespace config {

struct Token;
class ConfigNode;
class IArgument;

typedef std::vector<Token> ParsedDirectiveArgs;
typedef std::map<Token, ParsedDirectiveArgs> ParsedDirectiveMap;
typedef std::pair<Token, ParsedDirectiveArgs> ParsedDirectivePair;
typedef std::vector<ConfigNode> ConfigNodeVec;

typedef IArgument *ArgumentPtr;
typedef std::vector<ArgumentPtr> ArgumentVector;
typedef std::map<std::string, ArgumentVector> DirectiveMap;

} // namespace config
