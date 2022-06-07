#pragma once

#include<vector>

#include "Node.h"

namespace BitVector {
    template<typename PROFILER>
    class DynamicBitVector {
    public:
        DynamicBitVector() :
            root(NULL),
            length(0) {
            root = new Node();
        }
        //TODO constructor with existing bit vector

        inline bool access(const int index) noexcept {
            profiler.startAccess();
            const bool result = root->access(index);
            profiler.endAccess();
            return result;
        }

        /**
         * Inserts bit bit at index index.
         */
        inline void insertBit(int index, const bool bit) noexcept {
            profiler.startInsert();
            root = root->insertBit(index, bit, length);
            length++;
            profiler.endInsert();
        }

        inline void deleteBit(int index) noexcept {
            profiler.startDelete();
            const int ones = rankOne(length);
            root = root->deleteBit(index, length, ones);
            length--;
            profiler.endDelete();
        }

        inline void flipBit(int index) noexcept {
            profiler.startFlip();
            root->flipBit(index);
            profiler.endFlip();
        }

        inline int rank(const bool bit, const int index) noexcept {
            int result;
            profiler.startRank();
            if (bit) result = rankOne(index);
            result = rankZero(index);
            profiler.endRank();
            return result;
        }

        inline int select(const bool bit, const int j) noexcept {
            int result;
            profiler.startSelect();
            if (bit) result = selectOne(j);
            result = selectZero(j);
            profiler.endSelect();
            return result;
        }

    private:
        inline int rankOne(const int index) const noexcept {
            return root->rankOne(index);
        }

        inline int rankZero(const int index) const noexcept {
            return index - root->rankOne(index);
        }

        inline int selectOne(const int index) const noexcept {
            return root->selectOne(index);
        }

        inline int selectZero(const int index) const noexcept {
            return root->selectZero(index);
        }

    public:
        inline void printBitString() const noexcept {
            root->printBitString();
            std::cout << std::endl;
        }

        inline void printTree() const noexcept {
            root->printTree();
            std::cout << std::endl;
        }

        //the binary search tree
        Node* root;
        int length; //n

        PROFILER profiler;
    };
}