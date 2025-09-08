#pragma once

#include <string>
#include <sstream>

namespace utils {

/**
 * @enum LogLevel
 * @brief Defines the severity levels for log messages.
 */
enum LogLevel {
    TRACE,   //!< Fine-grained messages, typically for detailed debugging.
    DEBUG,   //!< Information useful for developers during debugging.
    INFO,    //!< Informational messages about application progress.
    WARNING, //!< Indicates a potential issue that is not a critical error.
    ERROR,   //!< An error that occurred but the application can recover from.
    FATAL    //!< A critical error that will likely lead to termination.
};

class Logger {
public:
    static void log(LogLevel, std::string const &);
    static void setLevel(LogLevel);

private:
    static LogLevel threshold_;
};

/**
 * @def LOG_MSG(level, msg)
 * @brief The base macro for logging. It constructs a string from the stream-like
 * input and passes it to the core logging function.
 * @param level The utils::LogLevel for the message.
 * @param msg A streamable expression (e.g., "User ID: " << id).
 */
#define LOG_MSG(level, msg)                                                                        \
    {                                                                                              \
        std::ostringstream oss;                                                                    \
        oss << msg;                                                                                \
        utils::Logger::log(level, oss.str());                                                      \
    }

/**
 * @def LOG_TRACE(msg)
 * @brief Logs a message with the TRACE severity level.
 * @details Use for highly detailed, step-by-step execution flow.
 */
#define LOG_TRACE(msg) LOG_MSG(utils::TRACE, msg)

/**
 * @def LOG_DEBUG(msg)
 * @brief Logs a message with the DEBUG severity level.
 * @details Use for information useful during development and debugging.
 */
#define LOG_DEBUG(msg) LOG_MSG(utils::DEBUG, msg)

/**
 * @def LOG_INFO(msg)
 * @brief Logs a message with the INFO severity level.
 * @details Use for general application progress and lifecycle events.
 */
#define LOG_INFO(msg) LOG_MSG(utils::INFO, msg)

/**
 * @def LOG_WARN(msg)
 * @brief Logs a message with the WARNING severity level.
 * @details Use for potential problems that do not stop program execution.
 */
#define LOG_WARN(msg) LOG_MSG(utils::WARNING, msg)

/**
 * @def LOG_ERROR(msg)
 * @brief Logs a message with the ERROR severity level.
 * @details Use for errors that the application can recover from.
 */
#define LOG_ERROR(msg) LOG_MSG(utils::ERROR, msg)

/**
 * @def LOG_FATAL(msg)
 * @brief Logs a message with the FATAL severity level.
 * @details Use for critical errors that prevent the application from continuing.
 */
#define LOG_FATAL(msg) LOG_MSG(utils::FATAL, msg)

} // namespace utils
