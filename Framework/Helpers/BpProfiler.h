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
        inline void startFwd() const noexcept {}
        inline void endFwd() const noexcept {}
        inline void startBwd() const noexcept {}
        inline void endBwd() const noexcept {}
        inline void startMinExcess() const noexcept {}
        inline void endMinExcess() const noexcept {}
        inline void startMinSelect() const noexcept {}
        inline void endMinSelect() const noexcept {}
        inline void startMinCount() const noexcept {}
        inline void endMinCount() const noexcept {}
        inline void print() const noexcept {}
    };

    class BasicProfiler {
    public:
        BasicProfiler() :
            insertTime(0),
            deleteTime(0),
            fwdTime(0),
            bwdTime(0),
            minExcessTime(0),
            minSelectTime(0),
            minCountTime(0),
            insertQueries(0),
            deleteQueries(0),
            fwdQueries(0),
            bwdQueries(0),
            minExcessQueries(0),
            minSelectQueries(0),
            minCountQueries(0) {
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

        inline void startFwd() noexcept {
            fwdTimer.restart();
        }

        inline void endFwd() noexcept {
            fwdTime += fwdTimer.getMicroseconds();
            fwdQueries++;
        }

        inline void startBwd() noexcept {
            bwdTimer.restart();
        }

        inline void endBwd() noexcept {
            bwdTime += bwdTimer.getMicroseconds();
            bwdQueries++;
        }

        inline void startMinExcess() noexcept {
            minExcessTimer.restart();
        }

        inline void endMinExcess() noexcept {
            minExcessTime += minExcessTimer.getMicroseconds();
            minExcessQueries++;
        }

        inline void startMinSelect() noexcept {
            minSelectTimer.restart();
        }

        inline void endMinSelect() noexcept {
            minSelectTime += minSelectTimer.getMicroseconds();
            minSelectQueries++;
        }

        inline void startMinCount() noexcept {
            minCountTimer.restart();
        }

        inline void endMinCount() noexcept {
            minCountTime += minCountTimer.getMicroseconds();
            minCountQueries++;
        }

        inline void print() const noexcept {
            std::cout << "Dynamic BP-Tree evaluation run. Average times per individual operation:" << std::endl;
            std::cout << "    InsertBit:  " << insertTime / insertQueries << " microseconds" << std::endl;
            std::cout << "    DeleteBit:  " << deleteTime / deleteQueries << " microseconds" << std::endl;
            std::cout << "    fwdSearch:  " << fwdTime / fwdQueries << " microseconds" << std::endl;
            std::cout << "    bwdSearch:  " << bwdTime / bwdQueries << " microseconds" << std::endl;
            std::cout << "    minExcess:  " << minExcessTime / minExcessQueries << " microseconds" << std::endl;
            std::cout << "    minSelect:  " << minSelectTime / minSelectQueries << " microseconds" << std::endl;
            std::cout << "    minCount:   " << minCountTime / minCountQueries << " microseconds" << std::endl;
        }

    private:
        Helpers::Timer insertTimer;
        Helpers::Timer deleteTimer;
        Helpers::Timer fwdTimer;
        Helpers::Timer bwdTimer;
        Helpers::Timer minExcessTimer;
        Helpers::Timer minSelectTimer;
        Helpers::Timer minCountTimer;

        size_t insertTime;
        size_t deleteTime;
        size_t fwdTime;
        size_t bwdTime;
        size_t minExcessTime;
        size_t minSelectTime;
        size_t minCountTime;

        double insertQueries;
        double deleteQueries;
        double fwdQueries;
        double bwdQueries;
        double minExcessQueries;
        double minSelectQueries;
        double minCountQueries;
    };
}