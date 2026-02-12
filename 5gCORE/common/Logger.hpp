#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    CRITICAL = 4
};

class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        currentLevel = level;
    }

    void log(LogLevel level, const std::string& component, const std::string& message) {
        if (level < currentLevel) {
            return;
        }

        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        std::string timeStr = oss.str();

        std::string levelStr = getLevelString(level);
        std::string colorCode = getColorCode(level);

        std::cout << colorCode << "[" << timeStr << "] [" << levelStr << "] [" << component << "] " 
                  << message << "\033[0m" << std::endl;
    }

    void debug(const std::string& component, const std::string& message) {
        log(LogLevel::DEBUG, component, message);
    }

    void info(const std::string& component, const std::string& message) {
        log(LogLevel::INFO, component, message);
    }

    void warning(const std::string& component, const std::string& message) {
        log(LogLevel::WARNING, component, message);
    }

    void error(const std::string& component, const std::string& message) {
        log(LogLevel::ERROR, component, message);
    }

    void critical(const std::string& component, const std::string& message) {
        log(LogLevel::CRITICAL, component, message);
    }

private:
    Logger() : currentLevel(LogLevel::INFO) {}

    std::string getLevelString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRIT";
            default: return "UNKNOWN";
        }
    }

    std::string getColorCode(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "\033[36m";      // Cyan
            case LogLevel::INFO: return "\033[32m";       // Green
            case LogLevel::WARNING: return "\033[33m";    // Yellow
            case LogLevel::ERROR: return "\033[31m";      // Red
            case LogLevel::CRITICAL: return "\033[35m";   // Magenta
            default: return "\033[0m";                     // Reset
        }
    }

    LogLevel currentLevel;
};

#endif // LOGGER_HPP
