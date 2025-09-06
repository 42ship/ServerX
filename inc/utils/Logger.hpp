#pragma once

#include <string>
#include <sstream>

#define LOG_TRACE(msg) LOG_MSG(utils::TRACE, msg)
#define LOG_DEBUG(msg) LOG_MSG(utils::DEBUG, msg)
#define LOG_INFO(msg) LOG_MSG(utils::INFO, msg)
#define LOG_WARN(msg) LOG_MSG(utils::WARN, msg)
#define LOG_ERROR(msg) LOG_MSG(utils::ERROR, msg)

namespace utils {

#define LOG_MSG(level, msg)                                                                        \
    {                                                                                              \
        std::ostringstream oss;                                                                    \
        oss << msg;                                                                                \
        utils::Logger::log(level, oss.str());                                                      \
    }

enum LogLevel {
    TRACE, /**< @brief Detailed trace information, typically for fine-grained debugging. */
    DEBUG, /**< @brief Debug-level messages, useful for development and troubleshooting. */
    INFO,  /**< @brief Informational messages that highlight the progress of the application. */
    WARN,  /**< @brief Indicates a potential problem or an unexpected situation that is not a
              critical error. */
    ERROR, /**< @brief A significant error occurred, but the application can continue to run. */
    FATAL  /**< @brief A critical error that will cause the application to terminate. */
};

class Logger {
public:
    static void log(LogLevel, std::string const &);

private:
    static LogLevel threashold_;
};

} // namespace utils
