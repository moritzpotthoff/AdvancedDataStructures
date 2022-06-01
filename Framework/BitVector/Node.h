#pragma once

#import <cstddef>
#import <bit>

#import "Definitions.h"

namespace BitVector {
    class InnerBitVector {
    public:
        InnerBitVector() {
            bits.reserve(w);
        }

        /**
         * Copies the blocks from old beginning at the middle to the new InnerBitVector, erases them from old.
         * @param old
         */
        InnerBitVector(InnerBitVector* old) {
            const int blockIndexStart = old->length / w / 2;
            std::copy(old->bits.begin() + blockIndexStart, old->bits.end(), std::back_inserter(this->bits));
            length = old->length - w * blockIndexStart;
            std::erase(old->bits.begin() + blockIndexStart, old->bits.end());
            old->length = std::min(old->length, w * blockIndexStart);
            old->bits.shrink_to_fit();
        }

        inline void insert(const int index, const bool bit) noexcept {
            if (length + 1 >= bits.capacity()) enlarge();
            bits.insert(index, bit);
            length++;
        }

        inline void deleteIndex(const int index) noexcept {
            bits.erase(index);
            if (length + 2 * w < bits.capacity()) //more than two words are unused
                shrink();
            length--;
        }

        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + w);
        }

        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - w);
        }

        inline int popcount() const noexcept {
            //TODO switch to int-based approach for this?
            int count = 0;
            for (int i = 0; i < bits.size(); i++) {
                if (bits[i]) count++;
            }
            return count;
        }

        std::vector<bool> bits;
        int length;
    };

    class Node {
    public:
        Node(InnerBitVector bitVector) {
            this->bitVector = bitVector;
            height = 0;
            //ones and num are not needed here.
        }

        Node() {
            height = 0;
            ones = 0;
            num = 0;
        }

        inline bool isLeaf() const noexcept{
            return leftChild == NULL && rightChild == NULL;
        }

        inline void insertBit(int index, bool bit, int length) noexcept {
            if (!isLeaf()) {
                if (i <= num) {
                    leftChild->insertBit(index, bit, num);
                    num++;
                    if (bit) ones++;
                } else {
                    rightChild->insertBit(index - num, bit, length - num);
                }
            } else {
                bitVector->insert(index, bit);
                if (length + 1 == 2 * w * w) {
                    //leaf overflow
                    //split bits into two halves,
                    InnerBitVector rightHalf(bitVector);
                    leftChild = Node(this->bitVector);
                    rightChild = Node(&rightHalf);
                    this->bitVector = NULL;
                    num = w * w;
                    ones = leftChild->bitVector.popcount();
                    height++;
                    //TODO assert that the size of the left child is w * w.
                }
            }
            //TODO perform potentially necessary rotations, update v
            this = rebalance();
        }

        inline void balanceFactor() {
            return height(leftChild) - height(rightChild);
        }

        /**
         * Rebalances the tree rooted at this, if necessary.
         *
         * @return The new root node, or this if nothing was rotated.
         */
        inline Node* rebalance() noexcept {
            const int balanceFactor = balanceFactor();
            if (balanceFactor > 1) {
                return rotateRight();
            } else if (balanceFactor < -1) {
                return rotateLeft();
            }
            return this;
        }

        inline Node* rotateLeft() noexcept {
            Node* previousLeftChild = rightChild->leftChild;
            Node* newRoot = rightChild;
            newRoot->leftChild = this;
            this-> rightChild = previousLeftChild;

            //adjust heights
            this->height = std::max(height(this->leftChild), height(this->rightChild)) + 1;
            newRoot->height = std::max(height(newRoot->leftChild), height(newRoot->rightChild)) + 1;

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
            this->height = std::max(height(this->leftChild), height(this->rightChild)) + 1;
            newRoot->height = std::max(height(newRoot->leftChild), height(newRoot->rightChild)) + 1;

            //update num and ones values
            this->num -= newRoot->num;
            this->ones -= newRoot->ones;

            return newRoot;
        }

        static int height(Node* node) {
            if (node == NULL) return 0;
            return node->height;
        }

        //TODO use union/variant
        Node* leftChild;
        Node* rightChild;
        InnerBitVector* bitVector;
        int height;

        //internal data
        int ones;
        int num;
    };
}
