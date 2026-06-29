#pragma once
#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <cstdio>
#include <cstdarg>

// Macros – use these in your code
#define LOG_DEBUG(fmt, ...) Logger::instance().log(LogLevel::DEBUG, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  Logger::instance().log(LogLevel::INFO,  fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger::instance().log(LogLevel::WARN,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::instance().log(LogLevel::ERRORR, fmt, ##__VA_ARGS__)

enum class LogLevel { DEBUG, INFO, WARN, ERRORR };

class Logger {
public:
    static Logger& instance();          // singleton
    void log(LogLevel level, const char* fmt, ...);   // printf‑style, non‑blocking
    void stop();                        // flush and stop worker thread
private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    struct LogEntry {
        std::string text;
    };

    void workerLoop();
    void enqueue(std::string msg);

    std::queue<LogEntry> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{ true };
    std::thread worker_;
};

#endif // LOGGER_H
