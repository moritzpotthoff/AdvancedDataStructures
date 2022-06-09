#pragma once

#include<vector>

#include "Node.h"
#include "../Helpers/BpProfiler.h"
#include "../Helpers/Asserts.h"

namespace BalancedParentheses {
    /**
     * A dynamic balanced parentheses representation of trees
     * that supports insertChild-, deleteNode, i-th-child-, parent-, subtreeSize-, and degree-queries.
     *
     * Implemented based on the description in
     *      Gonzalo Navarro. Compact Data Structures -- A Practical Approach.
     *      Cambridge University Press, 2016.
     *
     * Opposed to the source, we use 0-based indices B[0..length - 1].
     * Opposed to the lecture, we use rank queries with inclusive upper limits,
     * which yield excess queries that consider all entries up to and *including* the upper limit.
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

        /**
         * Inserts a child node. For the parent node v, the new node is inserted as the i-th
         * child (the leftmost child is the 1-th child). The new node becomes the parent of
         * the previously i-th to (i+k-1)-th children of v.
         *
         * @param v the index of the starting position for v
         * @param i the index in the sequence of children where the new node shall be inserted (1-indexed)
         * @param k the number of previous children of v starting from the previous i-th child that will become children of the new node.
         */
        inline void insertChild(const int v, const int i, const int k) noexcept {
            //std::cout << "Previously, tree is: " << std::endl;
            //printBitString();
            const int nChildren = children(v);
            //std::cout << "Inserting a child at node " << v << " with index " << i << ", getting " << k << " own children" << std::endl;
            //std::cout << "Number of children is " << nChildren << std::endl;
            //-1 to get 0-based indices
            const int openingPosition = ((i <= nChildren) ? child(v, i) : close(v));
            const int closingPosition = ((i + k <= nChildren) ? child(v, i + k) : close(v));
            //std::cout << "  Opening position is " << openingPosition << std::endl;
            //std::cout << "  Closing position is " << closingPosition << std::endl;
            insertBit(closingPosition, false);
            insertBit(openingPosition, true);
            //std::cout << "After insert, tree is: " << std::endl;
            //printBitString();
        }

        /**
         * Deletes the node that starts at position v.
         * All its children become children of the parent of v.
         *
         * @param v the index of the node that shall be deleted
         */
        inline void deleteNode(const int v) noexcept {
            AssertMsg(v > 0, "Cannot delete root node!");
            deleteBit(close(v));
            deleteBit(v);
        }

        /**
         * Computes the out-degree of a node.
         *
         * @param v the starting position of the node.
         * @return the degree.
         */
        inline int degree(const int v) const noexcept {
            //close(v) - 2 is the start index of the last child of v; each minimum excess in the range from after v until there corresponds to one child.
            const int closePos = close(v);
            //std::cout << "   Get children for " << v << " with closing index " << closePos << std::endl;
            return minCount(v, closePos - 2);
        }

        /**
         * A convenient alias for out-degree.
         *
         * @param v the starting position of the node.
         * @return the degree.
         */
        inline int children(const int v) const noexcept {
            return degree(v);
        }

        /**
         * Finds the starting position of the t-th child of node v.
         *
         * @param v the starting position of the parent node
         * @param t the index of the desired child in the sequence of all children of v, 1-indexed.
         * @return the starting position of the t-th child of v.
         */
        inline int child(int v, int t) const noexcept {
            //the t-th child starts at the t-th occurrence of the minimum excess in subtree of v.
            return minSelect(v, close(v) - 2, t) + 1;
        }

        /**
         * Finds the position of the parent of node v
         * @param v the node
         * @return the index of v's parent
         */
        inline int parent(int v) const noexcept {
            return enclose(v);
        }

        /**
         * Computes the subtree size of the subtree rooted at v.
         *
         * @param v the starting position of the subtree root
         * @return the size of the subtree, including v itself.
         */
        inline int subtreeSize(int v) const noexcept {
            //the subtree spans from v to close(v), i.e., there are close(v) - v + 1 parentheses, every two parentheses in there are one node
            return (close(v) - v + 1) / 2;
        }

    private:
        /**
         * Finds the closing parenthesis to an opening parenthesis at index i.
         *
         * @param i the index of the opening parenthesis
         * @return the closing index
         */
        inline int close(int i) const noexcept {
            AssertMsg(isOpening(i), "Searching for a closing bracket of a closing bracket.");
            //the closing bracket is at the first position where the total excess is one less than at i, as the opening bracket at i was closed.
            const int result = fwdSearch(i, -1);
            //std::cout << "Closing bracket for " << i << " is " << result  << std::endl;
            return result;
        }

        /**
         * For a position v of an opening parenthesis, finds the greatest
         * position k < v such that k belongs to a pair enclosing that of v.
         *
         * @param v the left parenthesis position that shall be enclosed
         * @return the left parenthesis position of the next enclosing pair
         */
        inline int enclose(const int v) const noexcept {
            AssertMsg(isOpening(v), "Searching for an enclosing bracket of a closing bracket.");
            //the relevant opening bracket is the first bracket to the left that has a difference in excess of -2 (the opening bracket at v, plus one more)
            return bwdSearch(v, -2);//no +1 (as in book) because of 0-indices
        }

        /**
         * Finds the first position j with excess(j) - excess(i) = d with j > i.
         *
         * @param i the start index
         * @param d the desired excess starting from i
         * @return the position j.
         */
        inline int fwdSearch(int i, int d) const noexcept {
            return root->fwdSearch(i, d, length);
        }

        /**
         * Finds the rightmost index j < i such that excess(j) - excess(i) = d
         *
         * @param i the start index (searching to the left of i)
         * @param d the desired excess starting from i
         * @return the position j
         */
        inline int bwdSearch(int i, int d) const noexcept {
            return root->bwdSearch(i, d, length);
        }

        /**
         * Finds the minimum excess in the interval [i,j]
         *
         * @param i the start index
         * @param j the end index
         * @return the minimum excess
         */
        inline int minExcess(int i, int j) const noexcept {
            return root->getMinExcess(i, j, length);
        }

        /**
         * Finds the position of the t-th occurrence of the minimum excess within [i..j].
         *
         * @param i the starting position of the range (inclusive)
         * @param j the end position of the range (inclusive)
         * @param t the index of the occurrence that shall be found
         * @return the position of the t-th occurrence of the minimum in the sequence of the excesses up to i, i+1, ..., j.
         */
        inline int minSelect(const int i, const int j, const int t) const noexcept {
            const int m = minExcess(i, j);
            return root->minSelect(i, j, t, length, m);
        }

        /**
         * Finds the number of times that the minimum excess from [i,j] occurs within that interval.
         *
         * @param i the starting position of the range (inclusive)
         * @param j the end position of the range (inclusive)
         * @return the number of occurrences of the minimum excess
         */
        inline int minCount(const int i, const int j) const noexcept {
            if (j <= i) return 0;
            const int m = minExcess(i, j);
            return root->minCount(i, j, length, m);
        }


    private:
        /**
         * Inserts bit bit at index index.
         *
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
         *
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
         *
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

        /**
         * An access query, only used for Asserts.
         * @param index the index that should be accessed
         * @return the bit at the given index
         */
        inline bool access(const int index) const noexcept {
            return root->access(index);
        }

        /**
         * Checks if the parenthesis at index is an opening one.
         * @param index
         * @return
         */
        inline bool isOpening(const int index) const noexcept {
            return access(index);
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