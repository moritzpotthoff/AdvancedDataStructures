#pragma once

#include <cstddef>
#include <bit>
#include <tuple>

#include "Definitions.h"
#include "InnerBitVector.h"
#include "InnerBitVectorByInt.h"
#include "../Helpers/Asserts.h"

namespace BalancedParentheses {
    /**
     * Node of the AVL-Tree that is used to represent the balanced parentheses tree.
     */
    template<typename INNER_BV>
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
            nodeHeight(0),
            totalExcess(0),
            minExcess(0),
            minTimes(0) {
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
            num(0),
            totalExcess(0),
            minExcess(0),
            minTimes(0)  {
            bitVector = new InnerBV();//TODO avoid this
        }

        /**
         * Inserts bit bit at index index
         *
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
                    //the new bit was inserted in the left subtree, adjust num and ones accordingly
                } else {
                    rightChild = rightChild->insertBit(index - num, bit, length - num);
                }
                //update the bp information
                recomputeExcessesInner();
            } else {
                //insert the bit into the inner bit vector of the leaf
                bitVector->insert(index, bit);
                recomputeExcessesLeaf();
                if (length + 1 == 2 * w * w) {
                    //leaf overflow, split leaf into two halves.
                    InnerBV* rightHalf = new InnerBV(bitVector);
                    //left half of the bits
                    leftChild = new Node(this->bitVector);
                    //right half of the bits. this node becomes the new inner node.
                    rightChild = new Node(rightHalf);
                    this->bitVector = NULL;//do not delete! is still used in leftChild
                    leftChild->recomputeExcesses();
                    rightChild->recomputeExcesses();
                    recomputeExcessesInner();
                    //adjust parameters
                    num = w * w;
                    ones = (w * w + leftChild->totalExcess) / 2;
                    nodeHeight++;
                    AssertMsg(leftChild->bitVector->bits.size() == w * w, "Left child has wrong size after split.");
                }
            }
            //while backtracking the path upwards, adjust the heights of all nodes as well
            nodeHeight = std::max(height(leftChild), height(rightChild)) + 1;
            //perform the necessary rotations in order to keep the tree balanced
            return rebalance();
        }

        /**
         * Delete the bit at the given index from the bit vector
         *
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

        /**
         * Performs a rank_1 query.
         *
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
         * Performs a forward search for the given parameters.
         */
        inline int fwdSearch(int i, int d, int length) const noexcept {
            int result;
            std::tie(std::ignore, result) = fwdSearchRecursive(i, d, length);
            return result;
        }

        /**
         * Performs a backward search for the given parameters.
         */
        inline int bwdSearch(int i, int d, int length) const noexcept {
            int result;
            std::tie(std::ignore, result) = bwdSearchRecursive(i, d, length);
            return result;
        }

        /**
         * Finds the minimum local excess (starting with index i) in the interval [i,j].
         *
         * @param i start index (inclusive)
         * @param j end index (inclusive)
         * @param length length of the subtree
         * @return (the minimum excess, the total excess)
         */
        inline std::pair<int, int> getMinExcess(const int i, const int j, const int length) const noexcept {
            if (i == 0 && j == length - 1) {
                //the interval is exactly the node interval, the minExcess is also the local minimum excess.
                return std::make_pair(minExcess, totalExcess);
            }
            if (isLeaf()) {
                return bitVector->minBlock(i, j);
            }
            if (j < num) {
                //interval is entirely in the left child
                return leftChild->getMinExcess(i, j, num);
            }
            if (i >= num) {
                int minRight, totalRight;
                std::tie(minRight, totalRight) = rightChild->getMinExcess(i - num, j - num, length - num);
                return std::make_pair(minRight, leftChild->totalExcess + totalRight);
            }
            //interval spans over both children, find the minimum from either sides.
            int minLeft, totalLeft, minRight, totalRight;
            std::tie(minLeft, totalLeft) = leftChild->getMinExcess(i, num - 1, num);
            std::tie(minRight, totalRight) = rightChild->getMinExcess(0, j - num, length - num);
            return std::make_pair(std::min(minLeft, minRight + totalLeft), totalLeft + totalRight);
        }

        /**
         * Finds the t-th occurrences of the minimum excess in the interval [i,j]
         *
         * @param i the start index
         * @param j the end index (inclusive)
         * @param t the index of the desired occurrence
         * @param length the length of the subtree
         * @param theMinExcess the desired excess
         * @return the index.
         */
        inline int minSelect(const int i, const int j, const int t, const int length, const int theMinExcess) const noexcept {
            AssertMsg(minCount(i, j, length, theMinExcess) >= t, "Trying to minSelect an occurrence that does not exist.");
            if (isLeaf()) {
                return bitVector->minSelectBlock(i, j, t, theMinExcess);
            }
            if (j < num) {
                //interval is only in left child, only search there
                return leftChild->minSelect(i, j, t, num, theMinExcess);
            }
            if (i >= num) {
                //interval is only in the right child, only search there
                return num + rightChild->minSelect(i - num, j - num, t, length - num, theMinExcess);
            }
            //interval spans over both left and right child. find out where the t-th occurrence is.
            int numberOfLeftOccurrences, leftSideExcess;
            std::tie(leftSideExcess, numberOfLeftOccurrences) = leftChild->minCountRec(i, num - 1, num, theMinExcess);
            if (t <= numberOfLeftOccurrences) {
                //the t-th occurrence of the minimum is in the left child
                return leftChild->minSelect(i, num - 1, t, num, theMinExcess);
            } else {
                //the t-th occurrence of the minimum is in the right child
                return num + rightChild->minSelect(0, j - num, t - numberOfLeftOccurrences, length - num, theMinExcess - leftSideExcess);
            }
        }

        /**
         * Performs a minCount query for the given range [i,j]
         *
         * @param i the start index
         * @param j the end index (inclusive)
         * @param t the index of the desired occurrence
         * @param length the length of the subtree
         * @param theMinimum the desired excess
         * @return the number of occurrences of the minimum excess
         */
        inline int minCount(const int i, const int j, const int length, const int theMinimum) const noexcept {
            int count;
            std::tie(std::ignore, count) = minCountRec(i, j, length, theMinimum);
            return count;
        }

    private:
        /**
         * Recomputes totalExcess, minExcess and minTimes for a node.
         */
        inline void recomputeExcesses() noexcept {
            if (isLeaf()) {
                recomputeExcessesLeaf();
            } else {
                recomputeExcessesInner();
            }
        }

        /**
         * Recomputes totalExcess, minExcess and minTimes for an inner node.
         */
        inline void recomputeExcessesInner() noexcept {
            totalExcess = leftChild->totalExcess + rightChild->totalExcess;
            minExcess = std::min(leftChild->minExcess, leftChild->totalExcess + rightChild->minExcess);
            if (leftChild->minExcess < leftChild->totalExcess + rightChild->minExcess) {
                //the left child has the minimum
                minTimes = leftChild->minTimes;
            } else if (leftChild->minExcess > leftChild->totalExcess + rightChild->minExcess) {
                //right side has the minimum
                minTimes = rightChild->minTimes;
            } else {
                //both minima are equal
                minTimes = leftChild->minTimes + rightChild->minTimes;
            }
        }

        /**
         * Recomputes totalExcess, minExcess and minTimes for a leaf.
         */
        inline void recomputeExcessesLeaf() noexcept {
            std::tie(totalExcess, minExcess, minTimes) = bitVector->recomputeExcesses();
        }

        /**
         * Performs a forward search.
         *
         * @param i the start index
         * @param d the desired excess
         * @param length the length of this subtree
         * @return (found excess, index)
         */
        inline std::pair<int, int> fwdSearchRecursive(int i, int d, int length) const noexcept {
            if (i == -1 && minExcess > d) {
                //excess d does not exist
                return std::make_pair(totalExcess, length);
            }
            if (isLeaf()) {
                //scan the bit vector of the leaf
                return bitVector->fwdBlock(i, d);
            }
            if (i >= num) {
                //we start in the right child, consider only the right child
                int newD, j;
                std::tie(newD, j) = rightChild->fwdSearchRecursive(i - num, d, length - num);
                return std::make_pair(newD, j + num);
            }
            //the search interval starts in the left child and spans into the right child
            int newD, j;
            //first, search in the left child
            std::tie(newD, j) = leftChild->fwdSearchRecursive(i, d, num);
            if (newD == d) {
                //result was found in left child, return it
                return std::make_pair(d, j);
            }
            int newD2;
            //result not found in the left child, need to also consider the right child
            std::tie(newD2, j) = rightChild->fwdSearchRecursive(-1, d - newD, length - num);
            return std::make_pair(newD + newD2, j + num);
        }

        /**
         * Performs a recursive backward search.
         * @param i
         * @param d
         * @param length
         * @return (foundExcess, position) -- if foundExcess == d, then position is the index
         *                                  otherwise, foundExcess is the total excess from i to the left and position is -1
         */
        inline std::tuple<int, int> bwdSearchRecursive(const int i, int d, const int length) const noexcept {
            //TODO does this help?
            if (d == 0 && bitVector->readBit(i) == false) {
                return std::make_pair(d, i);
            }
            if (i == length - 1 && d < -totalExcess + minExcess) {
                if (d == -totalExcess) return std::make_pair(d, 0);//just so met the desired excess at 0
                //we search through the entire block, and the desired excess can not exist (see book)
                return std::make_pair(-totalExcess, -1);
            }
            if (isLeaf()) {
                return bitVector->bwdBlock(i, d);
            }
            if (i < num) {
                //search interval is only in left child, search only here
                return leftChild->bwdSearchRecursive(i, d, num);
            }
            //search interval spans from right child into left child
            int excessFromRightChild, rightPosition;
            std::tie(excessFromRightChild, rightPosition) = rightChild->bwdSearchRecursive(i - num, d, length - num);
            if (excessFromRightChild == d) {
                //result already found in right child
                return std::make_pair(d, num + rightPosition);
            }
            //result not found in right child, need to also search in left child
            int excessFromLeftChild, leftPosition;
            std::tie(excessFromLeftChild, leftPosition) = leftChild->bwdSearchRecursive(num - 1, d - excessFromRightChild, num);
            return std::make_pair(excessFromRightChild + excessFromLeftChild, leftPosition);
        }

        /**
         * Counts the number of occurrences of theMinimum as local excess in the range [i,j]
         *
         * @param i start index
         * @param j end index (inclusive)
         * @param length subtree length
         * @param theMinimum desired minimum excess
         * @return (foundExcess, number of occurrences)
         */
        inline std::pair<int, int> minCountRec(const int i, const int j, const int length, const int theMinimum) const noexcept {
            if (i == 0 && j == length - 1) {
                //[i,j] is exactly the range covered by this node
                if (minExcess == theMinimum) return std::make_pair(totalExcess, minTimes);//minimum exists here
                if (minExcess > theMinimum) return std::make_pair(totalExcess, 0);//minimum does not exist here
            }
            if (isLeaf()) {
                return bitVector->minCount(i, j, theMinimum);
            }
            if (j < num) {
                //[i,j] is only in left child, search there
                return leftChild->minCountRec(i, j, num, theMinimum);
            }
            if (i >= num) {
                //[i,j] is only in the right child, search there
                return rightChild->minCountRec(i - num, j - num, length - num, theMinimum);
            }
            //[i,j] spans over some part of the left child and some part of the right child
            int leftCount, leftExcess;//leftExcess will be total excess from i to left end
            std::tie(leftExcess, leftCount) = leftChild->minCountRec(i, num - 1, num, theMinimum);
            //for the right child, the minimum has to be adjusted to be a valid local excess
            int rightCount, rightExcess;
            std::tie(rightExcess, rightCount) = rightChild->minCountRec(0, j - num, length - num, theMinimum - leftExcess);
            return std::make_pair(leftExcess + rightExcess, leftCount + rightCount);
        }

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
         * Rebalances the tree rooted at this, if necessary, updates all necessary fields.
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

            //update bp information
            this->recomputeExcessesInner();
            newRoot->recomputeExcessesInner();

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

            //recompute bp information
            this->recomputeExcessesInner();
            newRoot->recomputeExcessesInner();

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
                recomputeExcessesLeaf();
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
                    std::tie(leftChild, hasStolen, stolenBit) = leftChild->deleteRecursive(num - 1, num, ones, false);
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
            recomputeExcessesInner();
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
         * @param length the original length of the bit vector
         * @param bv the new bits
         * @param bvSize the size of the new bits
         * @param bvOnes number of 1 bits in the new bits
         * @return the new root for the subtree
         */
        inline void insertBitVector(int index, int length, InnerBV* bv, int bvSize, int bvOnes) noexcept {
            if (!isLeaf()) {
                if (index < num) {
                    //go left to insert
                    leftChild->insertBitVector(index, num, bv, bvSize, bvOnes);
                    num += bvSize;
                    ones += bvOnes;
                    recomputeExcessesInner();
                } else {
                    //go right
                    rightChild->insertBitVector(index - num, length - num, bv, bvSize, bvOnes);
                    recomputeExcessesInner();
                }
            } else {
                //actually insert the bits
                bitVector->insertBitVector(index, bv);
                recomputeExcessesLeaf();
            }
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
            std::cout << std::string(offset, ' ') << "Inner node with height " << nodeHeight << ", ones " << ones << ", num " << num << ", address " << this << std::endl;
            std::cout << std::string(offset, ' ') << "totalExcess " << totalExcess << ", minExcess" << minExcess << ", minTimes = " << minTimes << std::endl;
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

        inline size_t getSize() const noexcept {
            const size_t baseSize = CHAR_BIT * (sizeof(leftChild)
                                    + sizeof(rightChild)
                                    + sizeof(bitVector)
                                    + sizeof(nodeHeight)
                                    + sizeof(ones)
                                    + sizeof(num)
                                    + sizeof(totalExcess)
                                    + sizeof(minExcess)
                                    + sizeof(minTimes));
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

        //bit vector data
        int ones;
        int num;

        //bp data
        int totalExcess;//e in book, total excess(i,j) if [i,j] is the interval covered by the subtree rooted at this node
        int minExcess;//m in book, minimum local excess(i,k) for k <= j as above
        int minTimes;//n in book, number of times the minExcess occurs in [i,j] as above.
    };
}
