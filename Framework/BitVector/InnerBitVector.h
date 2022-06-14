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
            const size_t startIndex = old->length / 2;
            this->bits.assign(old->bits.begin() + startIndex, old->bits.end());
            length = old->length - startIndex;
            old->bits.erase(old->bits.begin() + startIndex, old->bits.end());
            old->length = std::min(old->length, startIndex);
            old->bits.shrink_to_fit();
        }

        inline void insert(const int index, const bool bit) noexcept {
            if (length + 1 >= bits.capacity()) enlarge();
            bits.insert(bits.begin() + index, bit);
            length++;
        }

        inline void insertBitVector(const int index, InnerBitVector* other, int otherSize) noexcept {
            bits.insert(bits.begin() + index, other->bits.begin(), other->bits.end());
            length += otherSize;
        }

        /**
         *
         * @param blockIndex
         * @param blockSize
         * @param newBits
         * @return number of ones in assigned bits
         */
        inline int insertBits(int blockIndex, size_t blockSize, std::vector<bool>& newBits) noexcept {
            AssertMsg(bits.size() == 0, "Tried to initialize-insert bits into non-empty leaf.");
            const size_t bitStart = blockIndex * blockSize;
            size_t blockLength = blockSize;
            if (newBits.size() - (bitStart + blockLength) < blockSize) {
                //this is the last block, give it all the bits.
                blockLength = newBits.size() - bitStart;
            }
            this->bits.assign(newBits.begin() + bitStart, newBits.begin() + bitStart + blockLength);
            length = bits.size();
            return popcount();
        }

        inline bool deleteIndex(const size_t index) noexcept {
            if (index >= length) return false;
            if (length == 0) return false;
            const bool bit = bits[index];
            bits.erase(bits.begin() + index);
            if (length + 2 * w < bits.capacity()) //more than two words are unused
                shrink();
            length--;
            return bit;
        }

        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + w);
        }

        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - w);
        }

        inline bool flipBit(const int index) noexcept {
            bits[index] = !bits[index];
            return !bits[index];//TODO use flip()
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
            //TODO actually use popcount
            if (j == 0) return 0;
            int p = 0;
            int count = 0;
            while (true) {
                if (bits[p]) count++;
                if (count == j) return p;
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

        inline void free() noexcept {
            std::vector<bool>().swap(bits);
            delete this;
        }

        std::vector<bool> bits;
        size_t length;
    };
}
