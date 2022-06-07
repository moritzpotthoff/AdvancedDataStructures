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

        inline int rankOne(int index) const noexcept {
            return root->rankOne(index);
        }

        inline int rankZero(int index) const noexcept {
            return index - root->rankOne(index);
        }

        inline bool access(int index) const noexcept {
            return root->access(index);
        }


        /**
         * Inserts bit bit at index index.
         */
        inline void insertBit(int index, bool bit) {
            std::cout << "Inserting bit " << bit << " at index " << index << std::endl;
            root = root->insertBit(index, bit, length);
            length++;
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