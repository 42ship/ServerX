#include "utils/Logger.hpp"
#include <ostream>
#include <iostream>
#include <ctime>

namespace utils {

#ifdef LOGLEVEL
LogLevel Logger::threshold_ = LOGLEVEL;
#else
LogLevel Logger::threshold_ = WARNING;
#endif

namespace {

#define RESET "\033[0m"
#define GRAY "\033[90m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define PURPLE "\033[35m"

char const *getTimeStamp() {
    static char buffer[80];
    std::time_t result = std::time(NULL);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&result));
    return buffer;
}

} // namespace

void Logger::log(LogLevel lvl, const std::string &msg) {
    if (lvl < threshold_)
        return;

    std::ostream *out = &std::cout;
    std::string s;
    std::string color;
    // clang-format off
    switch (lvl) {
    case TRACE: s = "TRACE";    color = GRAY;   break;
    case DEBUG: s = "DEBUG";    color = GRAY;   break;
    case INFO:  s = "INFO";     color = GREEN;  break;
    case WARNING:  s = "WARNING";     color = YELLOW; break;
    case ERROR: s = "ERROR";    color = RED;    out = &std::cerr;   break;
    case FATAL: s = "FATAL";    color = PURPLE; out = &std::cerr;   break;
    }
    // clang-format on
    *out << "[" << getTimeStamp() << "] " << color << "[" << s << "]" RESET " " << msg << std::endl;
}

void Logger::setLevel(LogLevel lvl) {
    threshold_ = lvl;
}

} // namespace utils
