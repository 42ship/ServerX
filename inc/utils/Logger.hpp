#pragma once

#include <sstream>
#include <string>

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

#ifndef LOGLEVEL
#define LOGLEVEL TRACE
#endif

#ifdef DISABLE_LOGGING
#define LOG_MSG(level, msg) (void)0
#else

/**
* @def LOG_MSG(level, msg)
* @brief Base macro. Uses a do-while(0) loop to scope variables and
* a compile-time 'if' check to strip out low-level logs.
* * @details
* If 'level' is less than the compiled 'LOGLEVEL', the compiler's
* optimizer (Dead Code Elimination) will completely remove the
* 'if' block. This prevents the expensive std::ostringstream
* construction and message evaluation from ever happening.
*/
#define LOG_MSG(level, msg)                                                                        \
    do {                                                                                           \
        if (level >= utils::LOGLEVEL) {                                                            \
            std::ostringstream oss;                                                                \
            oss << msg;                                                                            \
            utils::Logger::log(level, oss.str());                                                  \
        }                                                                                          \
    } while (0)

#endif

#define LOG_TRACE(msg) LOG_MSG(utils::TRACE, msg)
#define LOG_DEBUG(msg) LOG_MSG(utils::DEBUG, msg)
#define LOG_INFO(msg) LOG_MSG(utils::INFO, msg)
#define LOG_WARN(msg) LOG_MSG(utils::WARNING, msg)
#define LOG_ERROR(msg) LOG_MSG(utils::ERROR, msg)
#define LOG_FATAL(msg) LOG_MSG(utils::FATAL, msg)

} // namespace utils
