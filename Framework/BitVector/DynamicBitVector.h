#pragma once

#include<vector>

#include "Node.h"

namespace BitVector {
    /**
     * A dynamic bit vector that supports insert-, delete-, flip-, access-, rank- and select-queries.
     *
     * Implemented based on the description in
     *      Gonzalo Navarro. Compact Data Structures -- A Pracitcal Approach.
     *      Cambridge University Press, 2016.
     *
     * Opposed to the source, we use 0-based indices B[0..length - 1] and
     * our rank queries use exclusive upper limits.
     *
     * @tparam PROFILER type of profiler that can be used for tuning.
     *          NoProfiler can be used to avoid overheads,
     *          BasicProfiler for basic profiling.
     */
    template<typename PROFILER>
    class DynamicBitVector {
    public:
        /**
         * Creates an empty bit vector.
         */
        DynamicBitVector() :
            length(0) {
            root = new Node();
        }
        //TODO constructor with existing bit vector

        /**
         * An access query.
         * @param index the index that should be accessed
         * @return the bit at the given index
         */
        inline bool access(const int index) noexcept {
            profiler.startAccess();
            const bool result = root->access(index);
            profiler.endAccess();
            return result;
        }

        /**
         * Inserts bit bit at index index.
         * @param index the target index
         * @param bit the new bit
         */
        inline void insertBit(int index, const bool bit) noexcept {
            profiler.startInsert();
            root = root->insertBit(index, bit, length);
            length++;
            profiler.endInsert();
        }

        /**
         * Deletes the bit at the given index
         * @param index the index
         */
        inline void deleteBit(int index) noexcept {
            profiler.startDelete();
            const int ones = rankOne(length);
            root = root->deleteBit(index, length, ones);
            length--;
            profiler.endDelete();
        }

        /**
         * Flips the bit at the given index
         * @param index the index
         */
        inline void flipBit(int index) noexcept {
            profiler.startFlip();
            root->flipBit(index);
            profiler.endFlip();
        }

        /**
         * Computes rank-queries, i.e., the number of occurences of bit
         * in B[0, index - 1]
         * @param bit the bit that is searched, for rank_0 or rank_1 queries
         * @param index the (exclusive) upper limit
         * @return rank_bit(0..index - 1)
         */
        inline int rank(const bool bit, const int index) noexcept {
            int result;
            profiler.startRank();
            if (bit) {
                result = rankOne(index);
            } else {
                result = rankZero(index);
            }
            profiler.endRank();
            return result;
        }

        /**
         * Computes select queries, i.e., the position of the j-th
         * occurence of bit in the bit vector.
         * @param bit the bit that is searched, for select_0 or select_1 queries
         * @param j the number of the occurrence that is searched for
         * @return select_bit(j)
         */
        inline int select(const bool bit, const int j) noexcept {
            int result;
            profiler.startSelect();
            if (bit) {
                result = selectOne(j);
            } else {
                result = selectZero(j);
            }
            profiler.endSelect();
            return result;
        }

    private:
        //some helper functions
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
        //some helper functions
        inline void printBitString() const noexcept {
            root->printBitString();
            std::cout << std::endl;
        }

        inline void printTree() const noexcept {
            root->printTree();
            std::cout << std::endl;
        }

        //for testing
        inline std::vector<bool> getBitString() const noexcept {
            std::vector<bool> result(0);
            root->getBitString(&result);
            return result;
        }

        //the root of the binary search tree for the bit vector
        Node* root;
        int length;

        //profiler, used only for tuning
        PROFILER profiler;
    };
}