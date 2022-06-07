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
            const size_t startIndex = old->length / 2;
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

        inline void deleteIndex(const size_t index) noexcept {
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

        inline bool readBit(const int index) const noexcept {
            return bits[index];
        }

        /**
         * Executes select queries on bit vectors using popcount
         * @param j
         * @return the index of the j-th one in the bit vector
         */
        inline int selectOne(const int j) const noexcept {
            std::cout << "  In inner bit vector, selectOne " << j << std::endl;
            printBitString();
            //TODO actually use popcount
            if (j == 0) return 0;
            int p = 0;
            int count = 0;
            while (true) {
                if (bits[p]) count++;
                if (count == j) {
                    std::cout << "  Bit vector selectOne result is " << p << std::endl;
                    return p;
                }
                p++;
            }
        }

        /**
         * Executes select queries on bit vectors using popcount
         * @param j
         * @return the index of the j-th zero in the bit vector
         */
        inline int selectZero(const int j) const noexcept {
            //TODO actually use popcount
            if (j == 0) return 0;
            int p = 0;
            int count = 0;
            while (true) {
                if (!bits[p]) count++;
                if (count == j) return p;
                p++;
            }
        }

        inline int popcount() const noexcept {
            //TODO switch to uint64_t-based approach for this?
            int count = 0;
            for (size_t i = 0; i < bits.size(); i++) {
                if (bits[i]) count++;
            }
            return count;
        }

        /**
         * Returns the number of 1s in the bit vector up to the exclusive upper limit
         * @param upperLimit (exclusive!)
         * @return
         */
        inline int popcount(size_t upperLimit) const noexcept {
            //TODO switch to uint64_t-based approach for this?
            int count = 0;
            for (size_t i = 0; i < upperLimit; i++) {
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
        size_t length;
    };
}
