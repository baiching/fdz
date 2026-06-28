#include "logger.h"
#include <iostream>
#include <chrono>

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

Logger::Logger() : worker_(&Logger::workerLoop, this) {}

Logger::~Logger() { stop(); }

void Logger::stop() {
    running_.store(false);
    cv_.notify_one();
    if (worker_.joinable())
        worker_.join();
}

void Logger::log(LogLevel level, const char* fmt, ...) {
    // Format the message into a buffer (non‑blocking part)
    char buffer[4096];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    // Prepend timestamp and level (optional)
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%H:%M:%S", std::localtime(&tt));

    std::string msg = "[" + std::string(timebuf) + "." + std::to_string(ms.count()) + "]";
    switch (level) {
    case LogLevel::DEBUG: msg += "[DEBUG] "; break;
    case LogLevel::INFO:  msg += "[INFO]  "; break;
    case LogLevel::WARN:  msg += "[WARN]  "; break;
    case LogLevel::ERRORR: msg += "[ERROR] "; break;
    }
    msg += buffer;

    enqueue(std::move(msg));
}

void Logger::enqueue(std::string msg) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push({ std::move(msg) });
    }
    cv_.notify_one();
}

void Logger::workerLoop() {
    while (running_.load() || !queue_.empty()) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]() {
            return !queue_.empty() || !running_.load();
            });
        while (!queue_.empty()) {
            auto entry = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            std::cout << entry.text << std::endl;   // actual I/O – blocks only here
            lock.lock();
        }
    }
}
