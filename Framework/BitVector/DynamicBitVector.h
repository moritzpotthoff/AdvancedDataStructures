#pragma once

#include "Node.h"

namespace BitVector {
    class DynamicBitVector {
    public:
        /**
         * Creates empty dynamic bit vector
         */
        DynamicBitVector() {

        }

        /**
         * Inserts bit bit at index index.
         */
        public void insertBit(int index, bool bit) {
            root.insertBit(index, bit, length);
        }

        //TODO constructor with existing bit vector


        //the binary search tree
        Node root;
        int length; //n
    };
}