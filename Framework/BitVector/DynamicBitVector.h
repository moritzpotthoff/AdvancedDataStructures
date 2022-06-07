#pragma once

#include<vector>

#include "Node.h"

namespace BitVector {
    class DynamicBitVector {
    public:
        DynamicBitVector() :
            root(NULL),
            length(0) {
            root = new Node();
        }
        //TODO constructor with existing bit vector

        inline bool access(const int index) const noexcept {
            return root->access(index);
        }

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

        /**
         * Inserts bit bit at index index.
         */
        inline void insertBit(int index, const bool bit) noexcept {
            std::cout << "Inserting bit " << bit << " at index " << index << std::endl;
            root = root->insertBit(index, bit, length);
            length++;
        }

        inline void deleteBit(int index) noexcept {
            std::cout << "Deleting bit at index " << index << std::endl;
            const int ones = rankOne(length);
            root = root->deleteBit(index, length, ones);
        }

        inline void flipBit(int index) noexcept {
            std::cout << "Flipping bit at index " << index << std::endl;
            root->flipBit(index);
        }

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
    };
}