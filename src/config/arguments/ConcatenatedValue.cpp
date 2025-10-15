#include "config/arguments/ConcatenatedValue.hpp"
#include "config/arguments/IArgument.hpp"
#include <cstddef>

namespace config {

ConcatenatedValue::~ConcatenatedValue() {
    for (size_t i = 0; i < variables_.size(); i++) {
        delete variables_[i];
    }
}

void ConcatenatedValue::add(IArgument *arg) {
    if (arg)
        variables_.push_back(arg);
}

ConcatenatedValue::ConcatenatedValue(const ConcatenatedValue &other) {
    variables_.reserve(other.variables_.size());
    for (size_t i = 0; i < other.variables_.size(); ++i) {
        variables_.push_back(other.variables_[i]->clone());
    }
}

ConcatenatedValue &ConcatenatedValue::operator=(ConcatenatedValue const &other) {
    if (this == &other)
        return *this;
    for (size_t i = 0; i < variables_.size(); ++i) {
        delete variables_[i];
    }
    variables_.reserve(other.variables_.size());
    for (size_t i = 0; i < other.variables_.size(); ++i) {
        variables_.push_back(other.variables_[i]->clone());
    }
    return *this;
}

std::string ConcatenatedValue::evaluate(http::HttpRequest const &ctx) const {
    std::string res;
    for (size_t i = 0; i < variables_.size(); i++) {
        if (variables_[i])
            res += variables_[i]->evaluate(ctx);
    }
    return res;
}

std::string ConcatenatedValue::getRawValue() const {
    std::string res;
    for (size_t i = 0; i < variables_.size(); i++) {
        if (variables_[i])
            res += variables_[i]->getRawValue();
    }
    return res;
}

IArgument *ConcatenatedValue::clone() const { return new ConcatenatedValue(*this); }

} // namespace config
