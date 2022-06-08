#include <chrono>

#include "Timer.h"

namespace BalancedParantheses {
    class NoProfiler {
    public:
        inline void startAccess() const noexcept {}
        inline void endAccess() const noexcept {}
        inline void startInsert() const noexcept {}
        inline void endInsert() const noexcept {}
        inline void startDelete() const noexcept {}
        inline void endDelete() const noexcept {}
        inline void startFlip() const noexcept {}
        inline void endFlip() const noexcept {}
        inline void startRank() const noexcept {}
        inline void endRank() const noexcept {}
        inline void startSelect() const noexcept {}
        inline void endSelect() const noexcept {}
        inline void print() const noexcept {}
    };

    class BasicProfiler {
    public:
        BasicProfiler() :
                accessTime(0),
                insertTime(0),
                deleteTime(0),
                flipTime(0),
                rankTime(0),
                selectTime(0),
                accessQueries(0),
                insertQueries(0),
                deleteQueries(0),
                flipQueries(0),
                rankQueries(0),
                selectQueries(0)
            {}

        inline void startAccess() noexcept {
            accessTimer.restart();
        }

        inline void endAccess() noexcept {
            accessTime += accessTimer.getMicroseconds();
            accessQueries++;
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

        inline void startFlip() noexcept {
            flipTimer.restart();
        }

        inline void endFlip() noexcept {
            flipTime += flipTimer.getMicroseconds();
            flipQueries++;
        }

        inline void startRank() noexcept {
            rankTimer.restart();
        }

        inline void endRank() noexcept {
            rankTime += rankTimer.getMicroseconds();
            rankQueries++;
        }

        inline void startSelect() noexcept {
            selectTimer.restart();
        }

        inline void endSelect() noexcept {
            selectTime += selectTimer.getMicroseconds();
            selectQueries++;
        }

        inline void print() const noexcept {
            std::cout << "Dynamic Bit Vector evaluation run. Average times per query:" << std::endl;
            std::cout << "    Access: " << accessTime / accessQueries << " microseconds" << std::endl;
            std::cout << "    Insert: " << insertTime / insertQueries << " microseconds" << std::endl;
            std::cout << "    Delete: " << deleteTime / deleteQueries << " microseconds" << std::endl;
            std::cout << "    Flip:   " << flipTime / flipQueries << " microseconds" << std::endl;
            std::cout << "    Rank:   " << rankTime / rankQueries << " microseconds" << std::endl;
            std::cout << "    Select: " << selectTime / selectQueries << " microseconds" << std::endl;
        }

    private:
        Helpers::Timer accessTimer;
        Helpers::Timer insertTimer;
        Helpers::Timer deleteTimer;
        Helpers::Timer flipTimer;
        Helpers::Timer rankTimer;
        Helpers::Timer selectTimer;

        size_t accessTime;
        size_t insertTime;
        size_t deleteTime;
        size_t flipTime;
        size_t rankTime;
        size_t selectTime;

        double accessQueries;
        double insertQueries;
        double deleteQueries;
        double flipQueries;
        double rankQueries;
        double selectQueries;
    };
}