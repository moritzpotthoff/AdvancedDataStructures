#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"

#define wordSize 64 //64 bits per uint64_t

namespace BitVector {
    /**
     * Inspirations taken from https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit (06-29-2022)
     */
    class InnerBitVectorByInt {
    public:
        InnerBitVectorByInt() :
                words(1),
                length(0) {
            words[0] = 0ULL;
        }

        /**
         * Copies the blocks from old beginning at the middle to the new InnerBitVector, erases them from old.
         * @param old
         */
         InnerBitVectorByInt(InnerBitVectorByInt* old) :
                words(0),
                length(0) {
            const size_t startWord = old->length / wordSize / 2;
            //move the last half of the words to this, erase them from the other bv.
            this->words.assign(old->words.begin() + startWord, old->words.end());
            length = old->length - startWord * wordSize;
            old->words.erase(old->words.begin() + startWord, old->words.end());
            old->length = std::min(old->length, startWord * wordSize);
            old->words.shrink_to_fit();
        }

        inline void insert(const size_t index, const bool bit) noexcept {
            if (index == 0 && length == 0) {
                words[0] = 1ULL << (wordSize - 1);
                length++;
                return;
            }
            if (index == length) {
                //insert at the end of the bv
                const size_t bitPos = bitIndex(index);
                if (bitPos < wordSize) {
                    //there is still space in the last word, just set the bit and adjust length.
                    length++;
                    setBitTo(index, bit);
                } else {
                    //we need a new word
                    words.emplace_back(0);
                    length++;
                    setBitTo(index, bit);
                }
            } else {
                //insert in the middle of the bv, we need to shift the remaining bits.
                size_t wordIndex = word(index);
                //remember the last bit of the word (must be carried over to next word)
                bool carry = readBit(wordIndex * wordSize + wordSize - 1);
                //shift all bits after relevant index back by one
                shiftBackFromIndex(index);
                //insert the new bit
                setBitTo(index, bit);
                //push carried bit through the remaining bv
                while (wordIndex < word(length)) {
                    const bool newCarry = readBit(wordIndex * wordSize + wordSize - 1);
                    //shift everything to the right
                    setBitTo(wordIndex * wordSize, carry);//set first bit of the word to carry
                    carry = newCarry;
                    wordIndex++;
                }
                //insert last carried bit using the above base case.
                insert(length, carry);
            }
        }

        /**
         * Inserts the given bit vector other at the start or end (via index) of this bit vector.
         * @param index
         * @param other
         * @param otherSize
         */
        inline void insertBitVector(const int index, InnerBitVectorByInt* other, int otherSize) noexcept {
            AssertMsg(index == 0 || index == length, "Called insertBitVector with index not start or end.");
            if (index == 0) {
                //insert other at start
                const size_t otherWords = std::min(other->words.size(), other->length / wordSize + 1);
                words.insert(words.begin(), other->words.begin(), other->words.begin() + otherWords);
                if (other->length % wordSize != 0) {
                    //remove the gap between the two
                    shiftForwardFromIndex(otherWords * wordSize, wordSize - (other->length % wordSize));
                }
            } else {
                //insert other at end
                const size_t oldNumberOfWords = words.size();
                const size_t otherWords = std::min(other->words.size(), other->length / wordSize + 1);
                words.insert(words.end(), other->words.begin(), other->words.begin() + otherWords);
                if (length % wordSize != 0) {
                    //remove the gap between the two
                    shiftForwardFromIndex(oldNumberOfWords * wordSize, wordSize - (length % wordSize));
                }
            }
            length += otherSize;
        }

        /**
         * Used only to initialize the bv at the beginning.
         * Inserts the relevant portion of newBits into this.
         * @param blockIndex the index of the block of bits that shall be inserted
         * @param blockSize the size of each block
         * @param newBits the entire set of new bits, some of which shall be inserted here.
         * @return number of ones in assigned bits
         */
        inline int insertBits(int blockIndex, size_t blockSize, std::vector<bool>& newBits) noexcept {
            AssertMsg(length == 0, "Tried to initialize-insert bits into non-empty leaf.");
            const size_t bitStart = blockIndex * blockSize;
            size_t blockLength = blockSize;
            if (newBits.size() - (bitStart + blockLength) < blockSize) {
                //this is the last block, give it all the bits.
                blockLength = newBits.size() - bitStart;
            }
            //reserve enough size
            words.resize(blockLength / wordSize + 1);
            for (size_t i = 0; i < blockLength; i++) {
                setBitTo(i, newBits[bitStart + i]);
            }
            length = blockLength;
            return popcount();
        }

        /**
         * Deletes the bit at the given index.
         * @param index
         * @return
         */
        inline bool deleteIndex(const size_t index) noexcept {
            if (index >= length) return false;
            if (length == 0) return false;
            const bool bit = readBit(index);
            //from the back to the front, move bits forward.
            size_t wordIndex = word(length);
            bool carry = readBit(wordIndex * wordSize);//keep highest bit of the word
            while (wordIndex > word(index)) {
                words[wordIndex] = words[wordIndex] << 1;//shift left by one bit
                wordIndex--;
                carry = readBit(wordIndex * wordSize);
            }
            //inside the relevant word, delete bit and insert carry as rightmost bit.
            const uint64_t leftBitMask = ~((1ULL << (bitIndex(index) + 1)) - 1);//1..10..0 with exactly wordSize - bitIndex ones at the start.
            const uint64_t rightBitMask = (1ULL << bitIndex(index)) - 1;//0..01..1 with exactly bitIndex - 1 ones at the end.
            uint64_t leftHalf = words[word(index)] & leftBitMask;//keep everything in front of bitIndex
            uint64_t rightHalf = words[word(index)] & rightBitMask;//keep everything behind bitIndex
            words[word(index)] = leftHalf | (rightHalf << 1);//shift right half by one, assemble result
            setBitTo(word(index) + wordSize - 1, carry);

            //update capacity
            length--;
            if (length + 2 * wordSize < wordSize * words.capacity()) //more than two words are unused
                shrink();
            return bit;
        }

        /**
         * Flips the bit and returns the previous bit.
         * @param index
         * @return
         */
        inline bool flipBit(const int index) noexcept {
            words[word(index)] ^= 1ULL << bitIndex(index);
            return !readBit(index);
        }

        inline bool readBit(const int index) const noexcept {
            return (words[word(index)] >> (wordSize - 1 - bitIndex(index))) & 1ULL;
        }

        /**
         * Executes select queries on bit vectors using popcount
         * @param j
         * @return the index of the j-th one in the bit vector
         */
        inline int selectOne(const int j) const noexcept {
            if (j == 0) return 0;
            int index = 0, count = 0;
            int nextWordCount = std::popcount(words[0]);
            while (count + nextWordCount < j) {
                //the desired index is not in the next word, we can skip it entirely
                count += nextWordCount;
                index += wordSize;
                nextWordCount = std::popcount(words[word(index)]);
            }
            //remaining select query on the word that contains the desired index.
            while (true) {
                if (readBit(index)) count++;
                if (count == j) return index;
                index++;
            }
        }

        /**
         * Executes select queries on bit vectors using popcount
         * @param j
         * @return the index of the j-th zero in the bit vector
         */
        inline int selectZero(const int j) const noexcept {
            if (j == 0) return 0;
            int index = 0, count = 0;
            int relevantWordLength = std::min((size_t)wordSize, length);
            int nextWordCount = relevantWordLength - std::popcount(words[0]);
            while (count + nextWordCount < j) {
                //the desired index is not in the next word, we can skip it entirely
                count += nextWordCount;
                index += wordSize;
                relevantWordLength = std::min((size_t)wordSize, length - index);//for the last word, we need to ignore trailing zeros that dont belong to the bv
                nextWordCount = relevantWordLength - std::popcount(words[word(index)]);
            }
            //remaining select query on the word that contains the desired index.
            while (true) {
                if (!readBit(index)) count++;
                if (count == j) return index;
                index++;
            }
        }

        inline int popcount() const noexcept {
            int count = 0;
            for (size_t wordIndex = 0; wordIndex < word(length); wordIndex++) {
                count += std::popcount(words[wordIndex]);
            }
            return count;
        }

        /**
         * Returns the number of 1s in the bit vector up to the exclusive upper limit
         * @param upperLimit (exclusive!)
         * @return
         */
        inline int popcount(size_t upperLimit) const noexcept {
            size_t count = 0, index = 0;
            //fully covered words
            while (index + wordSize < upperLimit) {
                count += std::popcount(words[word(index)]);
                index += wordSize;
            }
            //remaining section at the end
            for (; index < upperLimit; index++) {
                if (readBit(index)) count++;
            }
            return count;
        }

        inline void free() noexcept {
            std::vector<uint64_t>().swap(words);
            delete this;
        }

    private:
        /**
         * Sets the bit at the desired index to bit.
         * @param index
         * @param bit
         */
        inline void setBitTo(size_t index, bool bit) noexcept {
            AssertMsg(index < length, "Tried to change bit outside the valid range.");
            const size_t bitPos = bitIndex(index);
            words[word(index)] = (words[word(index)] & ~(1ULL << (wordSize - 1 - bitPos))) | ((uint64_t)bit << (wordSize - 1 - bitPos));
        }

        /**
         * shifts all bits starting from (and including) index back by 1 step *within the word only*.
         * @param index
         */
        inline void shiftBackFromIndex(size_t index) noexcept {
            const uint64_t bitmask = (1ULL << (bitIndex(index) + 1)) - 1;//0..01..1 with exactly bitIndex ones at the end.
            uint64_t leftHalf = words[word(index)] & ~bitmask;//keep everything before bitIndex
            uint64_t rightHalf = words[word(index)] & bitmask;//keep everything starting from bitIndex
            words[word(index)] = leftHalf | (rightHalf >> 1);//shift right half by one, assemble result
        }

        /**
         * Shifts the bits starting from index forwards by delta bits, overwriting the delta bits
         * occuring directly in front of index.
         * @param index
         * @param delta
         */
        inline void shiftForwardFromIndex(size_t index, size_t delta) noexcept {
            size_t currentWord = words.size() - 1;
            uint64_t bitmaskUpper = ~(1ULL << (wordSize - delta + 1)) - 1;//delta ones at the start
            uint64_t carry = 0;
            while (currentWord > word(index)) {
                uint64_t newCarry = words[currentWord] & bitmaskUpper;
                words[currentWord] <<= delta;
                words[currentWord] |= carry;
                carry = newCarry;
                currentWord--;
            }
            //fill the gap with the last carry
            words[currentWord] |= carry;
        }

        inline void enlarge() noexcept {
            words.reserve(words.capacity() + wordSize);
        }

        inline void shrink() noexcept {
            words.reserve(words.capacity() - wordSize);
        }

        inline size_t word(size_t index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index / wordSize;
        }

        inline size_t bitIndex(size_t index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index % wordSize;
        }

    public:
        inline size_t getSize() const noexcept {
            //TODO length is not actually needed for this implementation. Either get rid of it or change implementation.
            return CHAR_BIT * (sizeof(length) + sizeof(words)) + words.size() * wordSize;
        }

        inline void printBitString(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ');
            for (size_t i = 0; i < length; i++) {
                std::cout << " " << readBit(i);
            }
            std::cout << "; length = " << length << std::endl;
        }

        inline void writeBitsToVector(std::vector<bool>* out) const noexcept {
            for (size_t i = 0; i < length; i++) {
                out->emplace_back(readBit(i));
            }
        }

        inline std::vector<bool> getBitString() const noexcept {
            std::vector<bool> result;
            writeBitsToVector(&result);
            return result;
        }

    private:
        std::vector<uint64_t> words;//all bits behind the first length bits must be 0! size must be maintained correctly.
    public:
        size_t length;
    };
}
