#pragma once

#include <cstddef>
#include <bit>
#include <tuple>

#include "Definitions.h"
#include "InnerBitVector.h"

namespace BitVector {
    class Node {
    public:
        Node(InnerBitVector* newBitVector) :
            leftChild(NULL),
            rightChild(NULL),
            bitVector(newBitVector),
            nodeHeight(0) {
            //ones and num are not needed here.
        }

        Node() :
            leftChild(NULL),
            rightChild(NULL),
            nodeHeight(0),
            ones(0),
            num(0) {
            bitVector = new InnerBitVector();
        }

        inline int access(int index) const noexcept {
            Node const* current = this;
            while (!current->isLeaf()) {
                if (index < current->num) {// < instead of <= in Navarro's book because we are 0-indexed
                    current = current->leftChild;
                } else {
                    index -= current->num;
                    current = current->rightChild;
                }
            }
            return current->bitVector->readBit(index);
        }

        inline int rankOne(int index) const noexcept {
            Node const* current = this;
            int currentOnes = 0;
            while (!current->isLeaf()) {
                if (index < current->num) {// < instead of <= in Navarro's book because we are 0-indexed
                    current = current->leftChild;
                } else {
                    index -= current->num;
                    currentOnes += current->ones;
                    current = current->rightChild;
                }
            }
            return currentOnes + current->popcount(index);
        }

        inline int selectOne(int j) const noexcept {
            if (j == 0) return 0;
            Node const* current = this;
            int pos = 0;
            while (!current->isLeaf()) {
                if (j <= current->ones) {
                    current = current->leftChild;
                } else {
                    j -= current->ones;
                    pos += current->num;
                    current = current->rightChild;
                }
            }
            const int p = current->bitVector->selectOne(j);
            return pos + p;
        }

        inline int selectZero(int j) const noexcept {
            if (j == 0) return 0;
            Node const* current = this;
            int pos = 0;
            while (!current->isLeaf()) {
                if (j <= current->num - current->ones) {
                    current = current->leftChild;
                } else {
                    j -= current->num - current->ones;
                    pos += current->num;
                    current = current->rightChild;
                }
            }
            const int p = current->bitVector->selectZero(j);
            return pos + p;
        }

        inline int popcount(int upperLimit) const noexcept {
            if (!isLeaf()) return -1; //error
            return bitVector->popcount(upperLimit);
        }

        inline bool isLeaf() const noexcept{
            return leftChild == NULL && rightChild == NULL;
        }

        inline bool flipBit(int index) noexcept {
            if (isLeaf()) return bitVector->flipBit(index);

            if (index < num) {
                const bool originalBit = leftChild->flipBit(index);
                originalBit ? ones-- : ones++;//a bit in the left subtree has flipped, adjust ones accordingly
                return originalBit;
            } else {
                return rightChild->flipBit(index - num);
            }
        }

        /**
         * Inserts bit bit at index index
         * @param index
         * @param bit
         * @param length -- see navarro book
         * @return the (new) parent node of the subtree that was changed in this recursive call of the function, to be used as child reference in the caller
         */
        inline Node* insertBit(int index, bool bit, int length) noexcept {
            if (!isLeaf()) {
                if (index < num) {// < instead of <= in Navarro's book because we are 0-indexed
                    leftChild = leftChild->insertBit(index, bit, num);
                    num++;
                    if (bit) ones++;
                } else {
                    rightChild = rightChild->insertBit(index - num, bit, length - num);
                }
            } else {
                bitVector->insert(index, bit);
                if (length + 1 == 2 * w * w) {
                    //leaf overflow
                    //split bits into two halves,
                    InnerBitVector* rightHalf = new InnerBitVector(bitVector);
                    leftChild = new Node(this->bitVector);
                    rightChild = new Node(rightHalf);
                    this->bitVector = NULL;
                    num = w * w;
                    ones = leftChild->bitVector->popcount();
                    nodeHeight++;
                    //TODO assert that the size of the left child is w * w.
                }
            }
            //while backtracking the path upwards, adjust the heights of all nodes as well
            nodeHeight = std::max(height(leftChild), height(rightChild)) + 1;
            return rebalance();
        }

        inline Node* deleteBit(int index, int length, int ones) noexcept {
            Node* newRoot;
            std::tie(newRoot, std::ignore, std::ignore) = deleteRecursive(index, length, ones, true);
            return newRoot;
        }

    private:
        inline int balanceFactor() const noexcept {
            return height(leftChild) - height(rightChild);
        }

        /**
         * Rebalances the tree rooted at this, if necessary.
         *
         * @return The new root node, or this if nothing was rotated.
         */
        inline Node* rebalance() noexcept {
            const int bf = balanceFactor();
            if (bf > 1) {
                return rotateRight();
            } else if (bf < -1) {
                return rotateLeft();
            }
            return this;
        }

        inline Node* rotateLeft() noexcept {
            Node* previousLeftChild = rightChild->leftChild;
            Node* newRoot = rightChild;
            newRoot->leftChild = this;
            this->rightChild = previousLeftChild;

            //adjust heights
            this->nodeHeight = std::max(height(this->leftChild), height(this->rightChild)) + 1;
            newRoot->nodeHeight = std::max(height(newRoot->leftChild), height(newRoot->rightChild)) + 1;

            //update num and ones values
            newRoot->num += this->num;
            newRoot->ones += this->ones;

            return newRoot;
        }

        inline Node* rotateRight() noexcept {
            Node* previousRightChild = leftChild->rightChild;
            Node* newRoot = leftChild;
            newRoot->rightChild = this;
            this->leftChild = previousRightChild;

            //adjust heights
            this->nodeHeight = std::max(height(this->leftChild), height(this->rightChild)) + 1;
            newRoot->nodeHeight = std::max(height(newRoot->leftChild), height(newRoot->rightChild)) + 1;

            //update num and ones values
            this->num -= newRoot->num;
            this->ones -= newRoot->ones;

            return newRoot;
        }

        /**
         *
         * @param index
         * @param length
         * @param bvOnes
         * @param underflow
         * @return three-tuple (v, hasDeleted, deletedBit)
         */
        inline std::tuple<Node*, bool, bool> deleteRecursive(int index, int length, int bvOnes, bool underflow) noexcept {
            bool hasDeleted, deletedBit, hasStolen, stolenBit;
            if (isLeaf()) {
                std::tie(hasDeleted, deletedBit) = deleteLeaf(index, length, underflow);
                return std::make_tuple(this, hasDeleted, deletedBit);
            }
            if (index < num) {
                std::tie(leftChild, hasDeleted, deletedBit) = leftChild->deleteRecursive(index, num, ones, underflow);
                if (!hasDeleted) return std::make_tuple(this, false, false);
                if (deletedBit) ones--;
                if (num == w * w / 2) {
                    //underflow, lefChild must be a leaf
                    std::tie(rightChild, hasStolen, stolenBit) = rightChild->deleteRecursive(1, length - w * w / 2, 0, false);
                    if (!hasStolen) {
                        //merge the leaves
                        Node* newLeaf = rightChild->insertBitVector(1, length - w * w / 2, leftChild->bitVector, w * w / 2 - 1, ones);
                        //TODO free this, leftChild and rightChild
                        return std::make_tuple(newLeaf, true, deletedBit);
                    }
                    leftChild->insertBit(w * w / 2, stolenBit, w * w / 2 - 1);
                    if (stolenBit) ones--;
                } else {
                    num--;
                }
            } else {
                std::tie(rightChild, hasDeleted, deletedBit) = rightChild->deleteRecursive(index - num, length - num, bvOnes - ones, underflow);
                if (!hasDeleted) return std::make_tuple(this, false, false);
                if (deletedBit) bvOnes--;
                if (length - num == w * w / 2) {
                    //underflow, rightChild must be a leaf
                    std::tie(leftChild, hasStolen, stolenBit) = leftChild->deleteRecursive(num, num, 0, false);
                    if (!hasStolen) {
                        //merge the leaves
                        Node* newLeaf = leftChild->insertBitVector(num + 1, num, rightChild->bitVector, w * w / 2 - 1, bvOnes - ones);
                        //TODO free this, leftChild and rightChild
                        return std::make_tuple(newLeaf, true, deletedBit);
                    }
                    rightChild->insertBit(1, stolenBit, w * w / 2 - 1);
                    num--;
                    if (stolenBit) ones--;
                }
            }

            Node* newRoot = rebalance();
            return std::make_tuple(newRoot, true, deletedBit);
        }

        /**
         *
         * @param index
         * @param length
         * @param underflow
         * @return tuple (hasDeleted, deletedBit)
         */
        inline std::pair<bool, bool> deleteLeaf(int index, int length, bool underflow) noexcept {
            if (length == w * w / 2 && !underflow) return std::make_pair(false, false);//forbidden underflow would occur, do not carry out deletion

            return std::make_pair(true, bitVector->deleteIndex(index));
        }

        inline Node* insertBitVector(int index, int length, InnerBitVector* bv, int bvSize, int bvOnes) noexcept {
            Node* current = this;
            while (!current->isLeaf()) {
                if (index < current->num) {
                    current->num += bvSize;
                    current->ones += bvOnes;
                    length = current->num;
                    current = current->leftChild;
                } else {
                    length -= current->num;
                    current = current->rightChild;
                }
            }
            //actually insert the bits
            //TODO make this nicer
            current->bitVector->insertBitVector(index, bv, bvSize);
            return current;
        }

    public:
        inline void printBitString() const noexcept {
            if (isLeaf()) {
                bitVector->printBitString();
            }
            if (leftChild != NULL) leftChild->printBitString();
            if (rightChild != NULL) rightChild->printBitString();
        }

        inline void printTree(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ') << "Inner node with height " << nodeHeight << ", ones" << ones << ", num " << num << ", address " << this << std::endl;
            if (leftChild != NULL) {
                std::cout << std::string(offset, ' ') << "--left child" << std::endl;
                leftChild->printTree(offset + 4);
            }
            if (rightChild != NULL) {
                std::cout << std::string(offset, ' ') << "--right child" << std::endl;
                rightChild->printTree(offset + 4);
            }
            if (isLeaf()) {
                //std::cout << std::string(offset, ' ') << "-- BIT VECTOR" << std::endl;
                bitVector->printBitString(offset + 4);
            }
        }

        static int height(Node* node) {
            if (node == NULL) return 0;
            return node->nodeHeight;
        }

        //TODO use union/variant
        Node* leftChild;
        Node* rightChild;
        InnerBitVector* bitVector;
        size_t nodeHeight;

        //internal data
        int ones;
        int num;
    };
}
