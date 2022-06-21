#pragma once

#include<vector>
#include<stack>

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
     * Opposed to the lecture, we use excess queries that consider all entries up to and *including* the upper limit.
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
            const int nChildren = children(v);
            AssertMsg(i <= nChildren + 1, "Invalid child index");
            const int openingPosition = ((i <= nChildren) ? child(v, i) : close(v));
            const int closingPosition = ((i + k <= nChildren) ? child(v, i + k) : close(v));
            insertBit(closingPosition, false);
            insertBit(openingPosition, true);
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
        inline int degree(const int v) noexcept {
            AssertMsg(isOpening(v), "Get degree of closing parenthesis");
            //close(v) - 2 is the start index of the last child of v; each minimum excess in the range from after v until there corresponds to one child.
            const int closePos = close(v);
            int result = minCount(v, closePos - 2);
            //printBitString();
            return result;
        }

        /**
         * A convenient alias for out-degree.
         *
         * @param v the starting position of the node.
         * @return the degree.
         */
        inline int children(const int v) noexcept {
            return degree(v);
        }

        /**
         * Finds the starting position of the t-th child of node v.
         *
         * @param v the starting position of the parent node
         * @param t the index of the desired child in the sequence of all children of v, 1-indexed.
         * @return the starting position of the t-th child of v.
         */
        inline int child(int v, int t) noexcept {
            AssertMsg(t <= children(v), "Try to get child that does not exist");
            //the t-th child is preceded by the t-th occurrence of the minimum excess in subtree of v.
            return minSelect(v, close(v) - 2, t) + 1;
        }

        /**
         * Finds the position of the parent of node v
         * @param v the node
         * @return the index of v's parent
         */
        inline int parent(int v) noexcept {
            return enclose(v);
        }

        /**
         * Computes the subtree size of the subtree rooted at v.
         *
         * @param v the starting position of the subtree root
         * @return the size of the subtree, including v itself.
         */
        inline int subtreeSize(int v) noexcept {
            //the subtree spans from v to close(v), i.e., there are close(v) - v + 1 parentheses, every two parentheses in there are one node
            return (close(v) - v + 1) / 2;
        }

        /**
         * Returns the index of the opening parenthesis for the node with the given number in an in order numbering of all nodes.
         * @param preorderNumber
         * @return
         */
        inline int getIndex(int preorderNumber) const noexcept {
            return root->selectOne(preorderNumber);
        }

    private:
        /**
         * Finds the closing parenthesis to an opening parenthesis at index i.
         *
         * @param i the index of the opening parenthesis
         * @return the closing index
         */
        inline int close(int i) noexcept {
            AssertMsg(isOpening(i), "Searching for a closing bracket of a closing bracket.");
            //the closing bracket is at the first position where the total excess is one less than at i, as the opening bracket at i was closed.
            const int result = fwdSearch(i, -1);
            return result;
        }

        /**
         * For a position v of an opening parenthesis, finds the greatest
         * position k < v such that k belongs to a pair enclosing that of v.
         *
         * @param v the left parenthesis position that shall be enclosed
         * @return the left parenthesis position of the next enclosing pair
         */
        inline int enclose(const int v) noexcept {
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
        inline int fwdSearch(int i, int d) noexcept {
            profiler.startFwd();
            const int result = root->fwdSearch(i, d, length);
            profiler.endFwd();
            return result;
        }

        /**
         * Finds the rightmost index j < i such that excess(j) - excess(i) = d
         *
         * @param i the start index (searching to the left of i)
         * @param d the desired excess starting from i
         * @return the position j
         */
        inline int bwdSearch(int i, int d) noexcept {
            profiler.startBwd();
            const int result = root->bwdSearch(i, d, length);
            profiler.endBwd();
            return result;
        }

        /**
         * Finds the minimum excess in the interval [i,j]
         *
         * @param i the start index
         * @param j the end index
         * @return the minimum excess
         */
        inline int minExcess(int i, int j) noexcept {
            int min;
            profiler.startMinExcess();
            std::tie(min, std::ignore) = root->getMinExcess(i, j, length);
            profiler.endMinExcess();
            return min;
        }

        /**
         * Finds the position of the t-th occurrence of the minimum excess within [i..j].
         *
         * @param i the starting position of the range (inclusive)
         * @param j the end position of the range (inclusive)
         * @param t the index of the occurrence that shall be found
         * @return the position of the t-th occurrence of the minimum in the sequence of the excesses up to i, i+1, ..., j.
         */
        inline int minSelect(const int i, const int j, const int t) noexcept {
            const int m = minExcess(i, j);
            profiler.startMinSelect();
            int result = root->minSelect(i, j, t, length, m);
            profiler.endMinSelect();
            return result;
        }

        /**
         * Finds the number of times that the minimum excess from [i,j] occurs within that interval.
         *
         * @param i the starting position of the range (inclusive)
         * @param j the end position of the range (inclusive)
         * @return the number of occurrences of the minimum excess
         */
        inline int minCount(const int i, const int j) noexcept {
            if (j < i) return 0;
            if (j == i) return 1;
            const int m = minExcess(i, j);
            profiler.startMinCount();
            const int result = root->minCount(i, j, length, m);
            profiler.endMinCount();
            return result;
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

        /**
         * Prints the degree of each node in preorder dfs order to the output file.
         * @param out
         */
        inline void printDegreesToFile(std::ofstream& out) noexcept {
            std::stack<int> stack;
            stack.emplace(0);
            while (!stack.empty()) {
                const int current = stack.top();
                stack.pop();
                //print the degree
                out << degree(current) << "\n";
                //push the children to the stack, in reverse order to explore children from left to right
                for (int i = children(current); i >= 1; i--) {
                    //TODO can we save some function calls here?
                    stack.emplace(child(current, i));
                }
            }
        }

        /**
         * Returns the size in bits
         * @return
         */
        inline size_t getSize() const noexcept {
            //length + rest
            return CHAR_BIT * sizeof(int) + root->getSize();
        }

        //the root of the binary search tree for the bit vector
        Node* root;
        int length;

        //profiler, used only for tuning
        PROFILER profiler;
    };
}