#pragma once

#include <map>
#include <string>
#include <vector>

namespace config {

struct Token;
class ConfigNode;
class IArgument;

typedef std::vector<Token> ParsedDirectiveArgs;
typedef std::map<std::string, ParsedDirectiveArgs> ParsedDirectiveMap;
typedef std::pair<std::string, ParsedDirectiveArgs> ParsedDirectivePair;
typedef std::vector<ConfigNode> ConfigNodeVec;

typedef IArgument *ArgumentPtr;
typedef std::vector<ArgumentPtr> ArgumentVector;
typedef std::map<std::string, ArgumentVector> DirectiveMap;

} // namespace config
