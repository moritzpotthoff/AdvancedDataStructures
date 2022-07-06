#pragma once

#include <cstddef>
#include <bit>
#include <tuple>
#include <math.h>

#include "Definitions.h"
#include "InnerBitVector.h"

namespace BitVector {
    /**
     * Node of the AVL-Tree that is used to represent the bit vector.
     */
    template<typename INNER_BV = InnerBitVector>
    class Node {
        using InnerBV = INNER_BV;
    public:
        /**
         * Creates a leaf node that contains the given bit vector
         * @param newBitVector the bit vector of the new node
         */
        Node(InnerBV* newBitVector) :
            leftChild(NULL),
            rightChild(NULL),
            bitVector(newBitVector),
            nodeHeight(0) {
            //ones and num are not needed here.
        }

        /**
         * Contains an empty bit vector.
         */
        Node() :
            leftChild(NULL),
            rightChild(NULL),
            nodeHeight(0),
            ones(0),
            num(0) {
            bitVector = new InnerBV();//TODO avoid this
        }

        /**
         * Creates a balanced binary tree with the blocks in [blockStart, blockEnd) as leaves and
         * assigns each block the relevant portion of size blockSize of bits from bits.
         *
         * @param blockStart
         * @param blockEnd
         * @param blockSize
         * @param bits
         * @return ones the number of ones in this block
         */
        inline int buildBinaryTree(int blockStart, int blockEnd, int blockSize, std::vector<bool>& bits) noexcept {
            if (blockEnd - blockStart == 1 || blockEnd == 0) {
                //this is a leaf.
                return bitVector->insertBits(blockStart, blockSize, bits);
            }
            const int numberOfLeaves = blockEnd - blockStart;
            const int subtreeHeight = ceil(log2(numberOfLeaves));
            const int fullBinaryLeaves = pow(2, subtreeHeight);
            int split;
            if (numberOfLeaves == fullBinaryLeaves) {
                //full binary tree
                split = ceil((double)blockStart + (double)numberOfLeaves / 2.0);
            } else if (ceil(log2(numberOfLeaves - fullBinaryLeaves / 2)) >= subtreeHeight - 2) {
                //if the left tree is a full binary tree, there are enough leaves left so that the right tree is balanced
                split = blockStart + pow(2, subtreeHeight - 1);
            } else {
                //there are not enough leaves left to have a full left tree. Give the right tree as much as is needed for balance and keep the rest left.
                const int leavesRight = fullBinaryLeaves / 2 / 2;
                const int leavesLeft = numberOfLeaves - leavesRight;
                split = blockStart + leavesLeft;
            }
            leftChild = new Node();
            rightChild = new Node();
            ones = leftChild->buildBinaryTree(blockStart, split, blockSize, bits);
            const int onesRight = rightChild->buildBinaryTree(split, blockEnd, blockSize, bits);
            num = std::min((split - blockStart) * blockSize, (int)bits.size());
            nodeHeight = subtreeHeight;
            return ones + onesRight;
        }

        /**
         * Performs an access query on the subtree rooted at this node.
         * @param index the index that shall be accessed
         * @return the bit B[index]
         */
        inline int access(int index) const noexcept {
            //traverse the tree until the relevant leaf is reached
            Node const* current = this;
            while (!current->isLeaf()) {
                if (index < current->num) {// < instead of <= in Navarro's book because we are 0-indexed
                    current = current->leftChild;
                } else {
                    index -= current->num;//num bits were skipped to the left
                    current = current->rightChild;
                }
            }
            //read the bit in the leaf
            return current->bitVector->readBit(index);
        }

        /**
         * Performs a rank_1 query.
         * @param index the index for which the rank is requested
         * @return the rank
         */
        inline int rankOne(int index) const noexcept {
            //traverse the tree
            Node const* current = this;
            int currentOnes = 0;//number of ones that was already skipped to the left
            while (!current->isLeaf()) {
                if (index < current->num) {// < instead of <= in Navarro's book because we are 0-indexed
                    current = current->leftChild;
                } else {
                    index -= current->num;
                    currentOnes += current->ones;
                    current = current->rightChild;
                }
            }
            //finish query in the leaf
            return currentOnes + current->popcount(index);
        }

        /**
         * Select_1 query.
         * @param j the number of bit 1 that is searched for.
         * @return the index of the j-th occurence of bit 1.
         */
        inline int selectOne(int j) const noexcept {
            if (j == 0) return 0;//by definition (Navarro)
            //traverse the tree
            Node const* current = this;
            int pos = 0;
            while (!current->isLeaf()) {
                if (j <= current->ones) {//here, use <= since we look for the j-th occurence
                    current = current->leftChild;
                } else {
                    j -= current->ones;
                    pos += current->num;
                    current = current->rightChild;
                }
            }
            //finish the query in the leaf
            return pos + current->bitVector->selectOne(j);
        }

        /**
         * Select_0 query.
         * @param j the number of bit 0 that is searched for.
         * @return the index of the j-th occurence of bit 0.
         */
        inline int selectZero(int j) const noexcept {
            if (j == 0) return 0;//by definition
            //traverse the tree
            Node const* current = this;
            int pos = 0;
            while (!current->isLeaf()) {
                if (j <= current->num - current->ones) {//use <= here, see above
                    current = current->leftChild;
                } else {
                    j -= current->num - current->ones;
                    pos += current->num;
                    current = current->rightChild;
                }
            }
            return pos + current->bitVector->selectZero(j);
        }

        /**
         * Flips the bit at index index
         * @param index
         * @return the bit that was at index before the flip.
         */
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
                    //the new bit was inserted in the left subtree, adjust num and ones accordingly
                    num++;
                    if (bit) ones++;
                } else {
                    rightChild = rightChild->insertBit(index - num, bit, length - num);
                }
            } else {
                //insert the bit into the inner bit vector of the leaf
                bitVector->insert(index, bit);
                if (length + 1 == 2 * w * w) {
                    //leaf overflow, split leaf into two halves.
                    InnerBV* rightHalf = new InnerBV(bitVector);
                    //left half of the bits
                    leftChild = new Node(this->bitVector);
                    //right half of the bits. this node becomes the new inner node.
                    rightChild = new Node(rightHalf);
                    this->bitVector = NULL;//do not delete! is still used in leftChild
                    //adjust parameters
                    num = w * w;
                    ones = leftChild->bitVector->popcount();
                    nodeHeight++;
                    //TODO assert that the size of the left child is w * w.
                }
            }
            //while backtracking the path upwards, adjust the heights of all nodes as well
            nodeHeight = std::max(height(leftChild), height(rightChild)) + 1;
            //perform the necessary rotations in order to keep the tree balanced
            return rebalance();
        }

        /**
         * Delete the bit at the given index from the bit vector
         * @param index
         * @param length the overall length of the subtree rooted at this node
         * @param ones the number of ones in the subtree rooted at this node
         * @return the new root of this subtree
         */
        inline Node* deleteBit(int index, int length, int ones) noexcept {
            Node* newRoot;
            std::tie(newRoot, std::ignore, std::ignore) = deleteRecursive(index, length, ones, true);
            return newRoot;
        }

    private:
        /**
         * Perform a popcount up to the given limit (exclusively) on the bit vector
         * @param upperLimit
         * @return the number of 1 bits before upperLimit
         */
        inline int popcount(int upperLimit) const noexcept {
            if (!isLeaf()) return -1; //error
            return bitVector->popcount(upperLimit);
        }

        /**
         * Checks if this node is a leaf
         * @return true, iff it is a leaf.
         */
        inline bool isLeaf() const noexcept{
            return leftChild == NULL && rightChild == NULL;
        }

        /**
         * Computes the balance factor for the AVL tree
         * @return the balance factor
         */
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
            //double rotations are not needed for the bit vector variant.
            return this;
        }

        /**
         * Performs a left rotation of the tree rooted at this.
         * @return the new root of this subtree.
         */
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

        /**
         * Performs a right rotation of the tree rooted at this.
         * @return the new root of this subtree.
         */
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
         * Recursive function for bit deletions
         * @param index the index of the bit that shall be deleted
         * @param length the length of the subtree rooted at this
         * @param bvOnes the number of ones in the subtree rooted at this
         * @param underflow if true, underflows are handled. If false, no deletion is carried out in case of an underflow.
         * @return three-tuple (newRoot, hasDeleted, deletedBit)
         */
        inline std::tuple<Node*, bool, bool> deleteRecursive(int index, int length, int bvOnes, bool underflow) noexcept {
            bool hasDeleted, deletedBit, hasStolen, stolenBit;
            if (isLeaf()) {
                std::tie(hasDeleted, deletedBit) = deleteLeaf(index, length, underflow);
                return std::make_tuple(this, hasDeleted, deletedBit);
            }
            if (index < num) {
                //delete in the left subtree
                std::tie(leftChild, hasDeleted, deletedBit) = leftChild->deleteRecursive(index, num, ones, underflow);
                if (!hasDeleted) return std::make_tuple(this, false, false);//no deletion, no bit can be removed from this tree
                if (deletedBit) ones--;
                if (num == w * w / 2) {
                    //underflow, lefChild must be a leaf.
                    //try to steal a bit from the other child to avoid merging
                    std::tie(rightChild, hasStolen, stolenBit) = rightChild->deleteRecursive(0, length - num, 0/*bvOnes - ones - (deletedBit ? 1 : 0)*/, false);
                    if (!hasStolen) {
                        //merge the leaves
                        rightChild->insertBitVector(0, length - num, leftChild->bitVector, num - 1, ones);
                        leftChild->free();
                        leftChild = NULL;
                        //delete rightChild;//TODO merge into this instead of child and also delete the child
                        return std::make_tuple(rightChild, true, deletedBit);
                    }
                    //insert the stolen bit at the correct index in the left child
                    leftChild->insertBit(num - 1, stolenBit, num - 1);
                    if (stolenBit) ones++;
                } else {
                    num--;
                }
            } else {
                //delete from the right child
                std::tie(rightChild, hasDeleted, deletedBit) = rightChild->deleteRecursive(index - num, length - num, bvOnes - ones, underflow);
                if (!hasDeleted) return std::make_tuple(this, false, false);
                if (deletedBit) bvOnes--;
                if (length - num == w * w / 2) {
                    //underflow, rightChild must be a leaf
                    //try to steal bit from the other child
                    std::tie(leftChild, hasStolen, stolenBit) = leftChild->deleteRecursive(num - 1, num, 0 /*ones*/, false);
                    if (!hasStolen) {
                        //merge the leaves
                        leftChild->insertBitVector(num, num, rightChild->bitVector, w * w / 2 - 1, bvOnes - ones);
                        //delete leftChild;//TODO merge into this instead of child and also delete the child
                        rightChild->free();
                        rightChild = NULL;
                        return std::make_tuple(leftChild, true, deletedBit);
                    }
                    //re-insert the stolen bit at the right position
                    rightChild->insertBit(0, stolenBit, w * w / 2 - 1);
                    //a bit was moved from left to right, adjust num and ones accordingly
                    num--;
                    if (stolenBit) ones--;
                }
            }
            //rebalance, if necessary.
            Node* newRoot = rebalance();
            return std::make_tuple(newRoot, true, deletedBit);
        }

        /**
         * Deletes a bit within a leaf
         * @param index the index of the bit that should be deleted
         * @param length the length of the bit vector in the leaf
         * @param underflow if true, underflows are allowed. Otherwise, in case of an underflow the deletion is not executed.
         * @return tuple (hasDeleted, deletedBit)
         */
        inline std::pair<bool, bool> deleteLeaf(int index, int length, bool underflow) noexcept {
            if (length == w * w / 2 && !underflow) return std::make_pair(false, false);//forbidden underflow would occur, do not carry out deletion

            return std::make_pair(true, bitVector->deleteIndex(index));
        }

        /**
         * Inserts an entire bit vector into this node at the given position.
         * @param index the position at which the new bit vector shall be inserted
         * @param length the length of the bit vector rooted at this node
         * @param bv the new bits
         * @param bvSize the size of the new bits
         * @param bvOnes number of 1 bits in the new bits
         */
        inline void insertBitVector(int index, int length, InnerBV* bv, int bvSize, int bvOnes) noexcept {
            AssertMsg(bvSize == (int)bv->length, "Wrong size in insert bit vector.");
            Node* current = this;
            while (!current->isLeaf()) {
                if (index < current->num) {
                    //go left to insert
                    current->num += bvSize;
                    current->ones += bvOnes;
                    length = current->num;
                    current = current->leftChild;
                } else {
                    //go right
                    index -= num;
                    length -= current->num;
                    current = current->rightChild;
                }
            }
            //actually insert the bits
            //TODO make this nicer
            //TODO why is this still small enough?
            current->bitVector->insertBitVector(index, bv, bvSize);
        }

        inline void free() noexcept {
            bitVector->free();
            delete this;
        }

    public:
        //some helper functions.
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
                bitVector->printBitString(offset + 4);
            }
        }

        /**
         * Helper to safely compute the height of a potentially non-existent node.
         * @param node the node
         * @return the height of the node, or 0 if node == NULL
         */
        static int height(Node* node) {
            if (node == NULL) return 0;
            return node->nodeHeight;
        }

        //for testing
        inline void getBitString(std::vector<bool>* result) const noexcept {
            if (isLeaf()) {
                bitVector->writeBitsToVector(result);
            } else {
                if (leftChild != NULL) {
                    leftChild->getBitString(result);
                }
                if (rightChild != NULL) {
                    rightChild->getBitString(result);
                }
            }
        }

        /**
         *
         * @return (total num in subtree, total ones in subtree, height)
         */
        inline std::tuple<int, int, int> validate() {
            if (isLeaf()) {
                AssertMsg(bitVector->length == bitVector->bits.size(), "BitVector length is false");
                AssertMsg(bitVector->length >= w * w / 2, "Bit Vector underflow");
                AssertMsg(bitVector->length <= 2 * w * w, "Bit Vector overflow");
                return std::make_tuple(bitVector->length, bitVector->popcount(), 0);
            }
            int leftNum, leftOnes, leftHeight;
            std::tie(leftNum, leftOnes, leftHeight) = leftChild->validate();
            AssertMsg(num == leftNum, "num is false");
            AssertMsg(ones == leftOnes, "ones is false");
            int rightNum, rightOnes, rightHeight;
            std::tie(rightNum, rightOnes, rightHeight) = rightChild->validate();
            return std::make_tuple(leftNum + rightNum, leftOnes + rightOnes, std::max(leftHeight, rightHeight) + 1);
        }

        inline size_t getSize() const noexcept {
            const size_t baseSize = CHAR_BIT * (sizeof(leftChild)
                                                + sizeof(rightChild)
                                                + sizeof(bitVector)
                                                + sizeof(nodeHeight)
                                                + sizeof(ones)
                                                + sizeof(num));
            if (isLeaf()) {
                return baseSize + bitVector->getSize();
            }
            return baseSize + leftChild->getSize() + rightChild->getSize();
        }

        //TODO use union/variant
        Node* leftChild;
        Node* rightChild;
        InnerBV* bitVector;
        size_t nodeHeight;

        //internal data
        int ones;
        int num;
    };
}
