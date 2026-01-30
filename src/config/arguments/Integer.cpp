#include "config/arguments/Integer.hpp"
#include "common/string.hpp"

namespace config {

Integer::Integer(size_t val) : iValue_(val), sValue_(utils::toString(val)) {}
std::string Integer::evaluate(http::Request const &) const { return sValue_; }
size_t Integer::getIntValue() const { return iValue_; }

} // namespace config
