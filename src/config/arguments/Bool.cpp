#include "config/arguments/Bool.hpp"
#include "common/string.hpp"
#include "config/internal/ConfigException.hpp"

namespace config {

Bool::Bool(bool value) : value_(value) {}

Bool::Bool(const char *value) {
    std::string s(value);
    init(s);
}

Bool::Bool(std::string const &value) { init(value); }

void Bool::init(std::string const &value) {
    std::string lower = value;
    utils::toLower(lower);

    if (lower == "true" || lower == "on" || lower == "yes" || lower == "1") {
        value_ = true;
    } else if (lower == "false" || lower == "off" || lower == "no" || lower == "0") {
        value_ = false;
    } else {
        throw ConfigError("Invalid boolean value: " + value);
    }
}

Bool::~Bool() {}

std::string Bool::evaluate(http::Request const &) const { return value_ ? "true" : "false"; }

ArgumentType Bool::getType() const { return ARG_BOOL; }

std::string Bool::getRawValue() const { return value_ ? "on" : "off"; }

IArgument *Bool::clone() const { return new Bool(value_); }

bool Bool::getValue() const { return value_; }

} // namespace config
