#pragma once

#include <chrono>

#include "Timer.h"

namespace BalancedParentheses {
    class NoProfiler {
    public:
        inline void startInsert() const noexcept {}
        inline void endInsert() const noexcept {}
        inline void startDelete() const noexcept {}
        inline void endDelete() const noexcept {}
        inline void startRank() const noexcept {}
        inline void endRank() const noexcept {}
        inline void print() const noexcept {}
    };

    class BasicProfiler {
    public:
        BasicProfiler() :
            insertTime(0),
            deleteTime(0),
            rankTime(0),
            insertQueries(0),
            deleteQueries(0),
            rankQueries(0) {
        }

        inline void startInsert() noexcept {
            insertTimer.restart();
        }

        inline void endInsert() noexcept {
            insertTime += insertTimer.getMicroseconds();
            insertQueries++;
        }

        inline void startDelete() noexcept {
            deleteTimer.restart();
        }

        inline void endDelete() noexcept {
            deleteTime += deleteTimer.getMicroseconds();
            deleteQueries++;
        }

        inline void startRank() noexcept {
            rankTimer.restart();
        }

        inline void endRank() noexcept {
            rankTime += rankTimer.getMicroseconds();
            rankQueries++;
        }

        inline void print() const noexcept {
            std::cout << "Dynamic BP-Tree evaluation run. Average times per query:" << std::endl;
            std::cout << "    Insert: " << insertTime / insertQueries << " microseconds" << std::endl;
            std::cout << "    Delete: " << deleteTime / deleteQueries << " microseconds" << std::endl;
            std::cout << "    Rank:   " << rankTime / rankQueries << " microseconds" << std::endl;
        }

    private:
        Helpers::Timer insertTimer;
        Helpers::Timer deleteTimer;
        Helpers::Timer rankTimer;

        size_t insertTime;
        size_t deleteTime;
        size_t rankTime;

        double insertQueries;
        double deleteQueries;
        double rankQueries;
    };
}