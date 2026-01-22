#pragma once

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <atomic>
#include <sstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

class Logger {
private:
    std::queue<std::string> queue;
    std::mutex mtx;
    std::condition_variable cv;
    std::thread worker;
    std::atomic<bool> running{ true };

    void process() {
        while (running || !queue.empty()) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return !queue.empty() || !running; });

            while (!queue.empty()) {
                std::string msg = std::move(queue.front());
                queue.pop();
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

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void push(const std::string& msg) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(msg);
        }
        cv.notify_one();
    }

    static std::string shortFile(const char* file) {
        std::string f = file ? file : "";
        auto pos = f.find_last_of("/\\");
        if (pos != std::string::npos) f = f.substr(pos + 1);
        return f;
    }

    // generic to string
    template <typename T>
    static std::string toString(const T& v) {
        std::ostringstream oss;
        oss << v;
        return oss.str();
    }

    static std::string toString(const glm::vec2& v) { return glm::to_string(v); }
    static std::string toString(const glm::vec3& v) { return glm::to_string(v); }
    static std::string toString(const glm::vec4& v) { return glm::to_string(v); }
    static std::string toString(const glm::mat3& v) { return glm::to_string(v); }
    static std::string toString(const glm::mat4& v) { return glm::to_string(v); }

    template <typename... Args>
    void log(const char* level,
        const char* file,
        int line,
        const char* func,
        const Args&... args)
    {
        std::string out;
        out += "[";
        out += level;
        out += "] ";
        out += shortFile(file);
        out += ":";
        out += std::to_string(line);
        out += " | ";
        out += func;
        out += ":: ";

        // expand args into string with +
        (void)std::initializer_list<int>{
            (out += toString(args), 0)...
        };

        out += "\n";
        push(out);
    }

public:
    static Logger& instance() {
        static Logger inst;
        return inst;
    }

    template <typename... Args>
    void info(const Args&... args) {
        log("INFO", __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION(), args...);
    }

    template <typename... Args>
    void warning(const Args&... args) {
        log("WARNING", __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION(), args...);
    }

    template <typename... Args>
    void error(const Args&... args) {
        log("ERROR", __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION(), args...);
    }

    template <typename... Args>
    void spacing(const Args&... args) {
        log("INFO", __builtin_FILE(), __builtin_LINE(), __builtin_FUNCTION(), args...);
    }
};

inline Logger& logger = Logger::instance();