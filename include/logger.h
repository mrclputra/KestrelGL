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
    void info(const std::string& msg) {
        log("INFO", msg);
    }
    void error(const std::string& msg) {
        log("ERROR", msg);
    }
    void warning(const std::string& msg) {
        log("WARNING", msg);
    }

private:
    void log(const std::string& level, const std::string& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        msgQueue.push("[" + level + "] - " + msg + "\n");
        cv.notify_one();
    }
};

// declare global
inline Logger& logger = Logger::instance();