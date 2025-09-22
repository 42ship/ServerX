#pragma once

#include <exception>
#include <string>

namespace config {

#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define NC "\033[0m"

class ConfigException : public std::exception {
private:
    std::string message_;

public:
    ConfigException(std::string const &message) : message_(message) {
    }
    virtual ~ConfigException() throw() {
    }
    char const *what(void) const throw() {
        return message_.c_str();
    }
};

class ConfigError : public ConfigException {
public:
    ConfigError(std::string const &message) : ConfigException(RED "Error: " NC + message) {
    }
};

class ConfigWarning : public ConfigException {
public:
    ConfigWarning(std::string const &message) : ConfigException(YELLOW "Warning: " NC + message) {
    }
};

void issue_warning(const std::string &msg);

} // namespace config
