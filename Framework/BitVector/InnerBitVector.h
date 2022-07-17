#pragma once

#include <cstddef>
#include <bit>

#include "../Definitions.h"

namespace BitVector {
    /**
     * A simple implementation of a dynamic bit vector with rank (popcount)- and select-support
     * based on a std::vector<bool>.
     * Should only be used to represent bits within nodes of a binary tree-based dynamic bit vector.
     */
    class InnerBitVector {
    public:
        /**
         * Creates an empty bit vector.
         */
        InnerBitVector() :
                bits(0) {
            bits.reserve(w);
        }

        /**
         * Creates a new InnerBitVector by splitting old.
         * Copies the bits from old beginning at the middle to the new InnerBitVector, erases them from old.
         * @param old the existing bit vector.
         */
        InnerBitVector(InnerBitVector* old) :
                bits(0){
            const size_t startIndex = old->bits.size() / 2;
            this->bits.assign(old->bits.begin() + startIndex, old->bits.end());
            old->bits.erase(old->bits.begin() + startIndex, old->bits.end());
            old->bits.shrink_to_fit();
        }

        /**
         * Inserts the entire given bit vector other into this bit vector at the beginning or end.
         *
         * @param index the index where other is inserted, either 0 or bits.size() (beginning or end).
         * @param other the bits to be inserted.
         */
        inline void insertBitVector(const int index, InnerBitVector* other) noexcept {
            bits.insert(bits.begin() + index, other->bits.begin(), other->bits.end());
        }

        /**
         * Inserts the relevant portion of bits from newBits into this bit vector
         * when construction an existing bit vector.
         * @param blockIndex the index of the block of bits that shall be inserted here
         * @param blockSize the size of each block
         * @param newBits the bits from which shall be inserted
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
            return popcount();
        }

        /**
         * Inserts a bit into the bit vector
         * @param index the target index
         * @param bit the new bit.
         */
        inline void insert(const int index, const bool bit) noexcept {
            if (bits.size() + 1 >= bits.capacity()) enlarge();
            bits.insert(bits.begin() + index, bit);
        }

        /**
         * Deletes a bit from the bit vector.
         * @param index the index which shall be deleted
         * @return the bit at that index.
         */
        inline bool deleteIndex(const size_t index) noexcept {
            if (index >= bits.size()) return false;
            if (bits.size() == 0) return false;
            const bool bit = bits[index];
            bits.erase(bits.begin() + index);
            if (bits.size() + 2 * w < bits.capacity()) //more than two words are unused
                shrink();
            return bit;
        }

        /**
         * Flips a bit.
         * @param index the index that shall be flipped.
         * @return the previous bit.
         */
        inline bool flipBit(const int index) noexcept {
            bits[index] = !bits[index];
            return !bits[index];
        }

        /**
         * Reads the bit at index index
         * @param index the index
         * @return the bit at index index
         */
        inline bool readBit(const int index) const noexcept {
            return bits[index];
        }

        /**
         * Executes select queries on bit vectors using a simple linear scan.
         * @param j the number of the desired occurrence
         * @return the index of the j-th one in the bit vector
         */
        inline int selectOne(const int j) const noexcept {
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
         * Executes select queries on bit vectors using a simple linear scan.
         * @param j the number of the desired occurrence
         * @return the index of the j-th zero in the bit vector
         */
        inline int selectZero(const int j) const noexcept {
            if (j == 0) return 0;
            int p = 0;
            int count = 0;
            while (true) {
                if (!bits[p]) count++;
                if (count == j) return p;
                p++;
            }
        }

        /**
         * Counts the number of ones in the bit vector using a simple linear scan.
         * @return the number of ones in the bv.
         */
        inline int popcount() const noexcept {
            int count = 0;
            for (size_t i = 0; i < bits.size(); i++) {
                if (bits[i]) count++;
            }
            return count;
        }

        /**
         * Returns the number of 1s in the bit vector up to the exclusive upper limit, i.e., performs a rank query.
         * @param upperLimit (exclusive!)
         * @return the number of ones.
         */
        inline int popcount(size_t upperLimit) const noexcept {
            int count = 0;
            for (size_t i = 0; i < upperLimit; i++) {
                if (bits[i]) count++;
            }
            return count;
        }

    private:
        /**
         * Reserves more space
         */
        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + w);
        }

        /**
         * Frees some space.
         */
        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - w);
        }

    public:
        //some helper functions.
        inline void printBitString(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ');
            for (size_t i = 0; i < bits.size(); i++) {
                std::cout << " " << bits[i];
            }
            std::cout << "; length = " << bits.size() << std::endl;
        }

        inline void writeBitsToVector(std::vector<bool>* out) const noexcept {
            std::copy(bits.begin(), bits.end(), std::back_inserter(*out));
        }

        inline std::vector<bool> getBitString() const noexcept {
            std::vector<bool> result;
            writeBitsToVector(&result);
            return result;
        }

        /**
         * Frees the memory used for this bv.
         */
        inline void free() noexcept {
            std::vector<bool>().swap(bits);
            delete this;
        }

        /**
         * Calculates the number of bits needed for this bv.
         * @return the number of bits.
         */
        inline size_t getSize() const noexcept {
            return CHAR_BIT * sizeof(bits) + bits.size();
        }

        inline size_t getLength() const noexcept {
            return bits.size();
        }

    public:
        std::vector<bool> bits;
    };
}
