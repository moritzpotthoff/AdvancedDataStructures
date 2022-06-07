#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"

namespace BitVector {
    class InnerBitVector {
    public:
        InnerBitVector() :
            bits(0),
            length(0) {
            bits.reserve(w);
        }

        /**
         * Copies the blocks from old beginning at the middle to the new InnerBitVector, erases them from old.
         * @param old
         */
        InnerBitVector(InnerBitVector* old) :
            bits(0),
            length(0) {
            //std::cout << "Called split constructor with old bit string" << std::endl;
            //old->printBitString();
            const int startIndex = old->length / 2;
            //std::cout << "Copying from begin to index " << startIndex << std::endl;
            this->bits.assign(old->bits.begin() + startIndex, old->bits.end());
            //std::cout << "Old: " << old->length << ", start Index is " << startIndex << std::endl;
            length = old->length - startIndex;
            old->bits.erase(old->bits.begin() + startIndex, old->bits.end());
            old->length = std::min(old->length, startIndex);
            old->bits.shrink_to_fit();
            //std::cout << "New bv length is " << length << ", new old length is " << old->length << std::endl;
            //std::cout << "New old bit string is" << std::endl;
            //old->printBitString();
            //std::cout << "Newly created bit string is" << std::endl;
            //printBitString();
        }

        inline void insert(const int index, const bool bit) noexcept {
            //std::cout << "Insert before:" << std::endl;
            //printBitString();
            if (length + 1 >= bits.capacity()) enlarge();
            bits.insert(bits.begin() + index, bit);
            length++;
            //std::cout << "Insert after:" << std::endl;
            //printBitString();
            //std::cout << std::endl;
        }

        inline void deleteIndex(const int index) noexcept {
            if (index > length) return;
            if (length == 0) return;
            bits.erase(bits.begin() + index);
            if (length + 2 * w < bits.capacity()) //more than two words are unused
                shrink();
            length--;
            //std::cout << "Delete after:" << std::endl;
            //printBitString();
            //std::cout << std::endl;
        }

        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + w);
        }

        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - w);
        }

        inline int popcount() const noexcept {
            //TODO switch to uint64_t-based approach for this?
            int count = 0;
            for (size_t i = 0; i < bits.size(); i++) {
                if (bits[i]) count++;
            }
            return count;
        }

        inline void printBitString(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ');
            for (size_t i = 0; i < bits.size(); i++) {
                std::cout << " " << bits[i];
            }
            std::cout << "; length = " << length << std::endl;
        }

        std::vector<bool> bits;
        int length;
    };

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

        inline bool isLeaf() const noexcept{
            return leftChild == NULL && rightChild == NULL;
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
                if (index <= num) {
                    std::cout << "  Inserting into left leaf" << std::endl;
                    leftChild = leftChild->insertBit(index, bit, num);
                    num++;
                    if (bit) ones++;
                } else {
                    std::cout << "  Inserting into right leaf" << std::endl;
                    rightChild = rightChild->insertBit(index - num, bit, length - num);
                }
            } else {
                bitVector->insert(index, bit);
                if (length + 1 == 2 * w * w) {
                    std::cout << "Split!" << std::endl;
                    //leaf overflow
                    //split bits into two halves,
                    InnerBitVector* rightHalf = new InnerBitVector(bitVector);
                    leftChild = new Node(this->bitVector);
                    rightChild = new Node(rightHalf);
                    //std::cout << "Right child has bits" << std::endl;
                    rightChild->printBitString();
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
                std::cout << "  Rotating right on node " << this << std::endl;
                return rotateRight();
            } else if (bf < -1) {
                std::cout << "  Rotating left on node " << this << std::endl;
                return rotateLeft();
            }
            std::cout << "  Not rotating on node " << this << std::endl;
            return this;
        }

        inline Node* rotateLeft() noexcept {
            std::cout << "BEFORE LEFT ROTATION, TREE IS " << std::endl;
            this->printTree();

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

            std::cout << "AFTER LEFT ROTATION, TREE FROM NEW ROOT IS:" << std::endl;
            newRoot->printTree();

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

        inline void printBitString() const noexcept {
            if (isLeaf()) {
                bitVector->printBitString();
            }
            if (leftChild != NULL) leftChild->printBitString();
            if (rightChild != NULL) rightChild->printBitString();
        }

        inline void printTree(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ') << "Inner node with height " << nodeHeight << ", address " << this << std::endl;
            if (leftChild != NULL) {
                std::cout << std::string(offset, ' ') << "--left child" << std::endl;
                leftChild->printTree(offset + 1);
            }
            if (rightChild != NULL) {
                std::cout << std::string(offset, ' ') << "--right child" << std::endl;
                rightChild->printTree(offset + 1);
            }
            if (isLeaf()) {
                //std::cout << std::string(offset, ' ') << "-- BIT VECTOR" << std::endl;
                bitVector->printBitString(offset + 1);
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
