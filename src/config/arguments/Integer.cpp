#include "config/arguments/Integer.hpp"
#include "utils/utils.hpp"

namespace config {

Integer::Integer(int val) : iValue_(val), sValue_(utils::toString(val)) {}
std::string Integer::evaluate(http::HttpRequest const &) const { return sValue_; }
int Integer::getIntValue() const { return iValue_; }

} // namespace config
