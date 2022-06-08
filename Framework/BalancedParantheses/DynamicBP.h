#pragma once

#include<vector>

#include "Node.h"
#include "../Helpers/BpProfiler.h"

namespace BalancedParantheses {
    /**
     * A dynamic balanced parantheses representation of trees
     * that supports insertChild-, deleteNode, i-th-child-, parent-, subtreeSize-, and degree-queries.
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
    class DynamicBP {
    public:
        /**
         * Creates a bp-tree consisting only of a root node
         */
        DynamicBP() :
            length(0) {
            root = new Node();
            //initially, the tree only has a root node, represented by (), or the bit sequence 10
            insertBit(0, true);
            insertBit(1, false);
        }

        inline void insertChild(int v, int i, int k) noexcept {
            const int p = (i <= children(v)) ? child(v, i) : close(v);
            const int q = (i + k <= children(v)) ? child(v, i + k) : close(v);
            insertBit(q, false);
            insertBit(p, true);
        }

        inline void deleteNode(int v) noexcept {
            deleteBit(close(v));
            deleteBit(v);
        }

        inline int degree(int v) const noexcept {
            return minCount(v, close(v) - 2);
        }

        inline int children(int v) const noexcept {
            //alias
            return degree(v);
        }

        inline int child(int v, int t) const noexcept {
            return minSelect(v, close(v) - 2, t) + 1;
        }

        inline int parent(int v) const noexcept {
            //TODO
            //return enclose(v);
            return 0;
        }

        inline int subtreeSize(int v) const noexcept {
            return (close(v) - v + 1) / 2;
        }

    private:
        inline int fwdSearch(int i, int d) const noexcept {
            return root->fwdSearch(i, d, length);
        }

        inline int bwdSearch(int i, int d) const noexcept {
            return root->bwdSearch(i, d, length);
        }

        inline int minExcess(int i, int j) const noexcept {
            return root->getMinExcess(i, j, length);
        }

        inline int close(int i) const noexcept {
            return fwdSearch(i, -1);//TODO check paramters because of 0-index
        }

        inline int minSelect(const int i, const int j, const int t) const noexcept {
            const int m = minExcess(i, j);
            return root->minSelect(i, j, t, length, m);
        }

        inline int minCount(const int i, const int j) const noexcept {
            const int m = minExcess(i, j);
            return root->minCount(i, j, length, m);
        }

        inline int enclose(const int v) const noexcept {
            return bwdSearch(v, -2) + 1;
        }

    private:
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
         * Computes rank-queries, i.e., the number of occurences of bit
         * in B[0, index - 1]
         * @param bit the bit that is searched, for rank_0 or rank_1 queries
         * @param index the (exclusive) upper limit
         * @return rank_bit(0..index - 1)
         */
        inline int rank(const bool bit, const int index) noexcept {
            int result;
            profiler.startRank();
            if (bit) result = rankOne(index);
            result = rankZero(index);
            profiler.endRank();
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

        //the root of the binary search tree for the bit vector
        Node* root;
        int length;

        //profiler, used only for tuning
        PROFILER profiler;
    };
}