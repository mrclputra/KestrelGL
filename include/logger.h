#pragma once

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>

class Logger {
private:
    std::queue<std::string> msgQueue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> running{ true };

	// the worker thread just keeps writing whatever comes in
    void process() {
        while (running || !msgQueue.empty()) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !msgQueue.empty() || !running; });

            while (!msgQueue.empty()) {
                std::string msg = msgQueue.front();
                msgQueue.pop();
                lock.unlock();

                std::cout << msg << std::flush;
                lock.lock();
            }
        }
    }

    Logger() {
        worker = std::thread(&Logger::process, this);
    }

    ~Logger() {
        running = false;
        cv.notify_one();
        if (worker.joinable()) worker.join();
    }

    // prevent copying
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

public:
    // get single instance
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    // push messages to queue
    // non-blocking
    void info(const std::string& msg,
        const char* file = __builtin_FILE(),
        int line = __builtin_LINE(),
        const char* func = __builtin_FUNCTION()) {
        log("INFO", msg, file, line, func);
    }
    void warning(const std::string& msg,
        const char* file = __builtin_FILE(),
        int line = __builtin_LINE(),
        const char* func = __builtin_FUNCTION()) {
        log("WARNING", msg, file, line, func);
    }
    void error(const std::string& msg,
        const char* file = __builtin_FILE(),
        int line = __builtin_LINE(),
        const char* func = __builtin_FUNCTION()) {
        log("ERROR", msg, file, line, func);
    }

private:
    void log(const std::string& level, const std::string& msg,
        const char* file, int line, const char* func)
    {
        std::string filename(file);
        auto pos = filename.find_last_of("/\\"); // handle both '/' and '\'
        if (pos != std::string::npos) {
            filename = filename.substr(pos + 1);
        }

        std::lock_guard<std::mutex> lock(mtx);
        std::string out = "[" + level + "] ";
        out += filename;
        out += ":";
        out += std::to_string(line);
        out += " | " + std::string(func) + ":: ";
        out += msg + "\n";
        msgQueue.push(std::move(out));
        cv.notify_one();
    }
};

// declare global
inline Logger& logger = Logger::instance();