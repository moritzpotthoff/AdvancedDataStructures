#pragma once
#include <chrono>

namespace Helpers {

    /**
     * Simple Timer used for the evaluation.
     */
    class Timer {
    public:
        Timer() : start(std::chrono::high_resolution_clock::now()) {}

        inline void restart() noexcept {
            start = std::chrono::high_resolution_clock::now();
        }

        inline size_t getMilliseconds() {
            std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        }

        inline size_t getMicroseconds() {
            std::chrono::time_point<std::chrono::high_resolution_clock> end = std::chrono::high_resolution_clock::now();
            return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        }

    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
    };
}