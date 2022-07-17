#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"

#define wordSize 64 //64 bits per uint64_t

namespace BalancedParentheses {
    /**
     * An implementation of a dynamic bit vector with rank (popcount)- and select-support.
     * Bits are represented using a sequence of uint64_t integers, each containing 64 of the original bits.
     * Global index i is at word i / 64, index i % 64.
     * Within words, the most significant bit is bit 0 (left-to-right).
     * Should only be used to represent bits within nodes of a binary tree-based dynamic bit vector.
     *
     * Some inspirations for bit access and manipulation were taken from
     * https://stackoverflow.com/questions/47981/how-do-you-set-clear-and-toggle-a-single-bit (06-29-2022)
     */
    class InnerBitVectorByInt {
    public:
        /**
         * Creates an empty bit vector.
         */
        InnerBitVectorByInt() :
                words(1),
                length(0) {
            words[0] = 0ULL;
        }

        /**
         * Creates a new InnerBitVector by splitting old.
         * Copies the bits from old beginning at the middle to the new bit vector, erases them from old.
         * @param old the existing bit vector.
         */
         InnerBitVectorByInt(InnerBitVectorByInt* old) :
                words(0),
                length(0) {
            const int startWord = old->length / wordSize / 2;
            //move the last half of the words to this, erase them from the other bv.
            this->words.assign(old->words.begin() + startWord, old->words.end());
            length = std::max(0, old->length - startWord * wordSize);
            old->words.erase(old->words.begin() + startWord, old->words.end());
            old->length = std::min(old->length, startWord * wordSize);
            old->words.shrink_to_fit();
        }

        /**
         * Inserts the given bit vector other at the start or end (via index) of this bit vector.
         * @param index the index where other is inserted, either 0 or length
         * @param other the bit vector to be inserted
         */
        inline void insertBitVector(const int index, InnerBitVectorByInt* other) noexcept {
            AssertMsg(index == 0 || index == length, "Called insertBitVector with index not start or end.");
            const int oldLength = length;
            if (index == 0) {
                //insert other at start
                int otherWords = other->length / wordSize;
                if (other->length % wordSize != 0) otherWords++;
                words.insert(words.begin(), other->words.begin(), other->words.begin() + otherWords);
                length += otherWords * wordSize;//temporarily adjust length for the following operations
                if (other->length % wordSize != 0) {
                    //remove the gap between the two regions
                    const int delta = wordSize - (other->length % wordSize);
                    shiftForwardFromIndex(otherWords * wordSize, delta);
                }
            } else {
                //insert other at end
                shrinkToFit();
                const int oldNumberOfWords = words.size();
                int otherWords = other->length / wordSize;
                if (other->length % wordSize != 0) otherWords++;
                words.insert(words.end(), other->words.begin(), other->words.begin() + otherWords);
                length += otherWords * wordSize;//temporarily adjust length for the following operations
                if (oldLength % wordSize != 0) {
                    //remove the gap between the two regions
                    const int delta = wordSize - (oldLength % wordSize);
                    shiftForwardFromIndex(oldNumberOfWords * wordSize, delta);
                }
            }
            length = oldLength + other->length;//actually set correct length
            //other will be deleted by the caller
        }

        /**
         * Inserts a bit into the bit vector
         * @param index the target index
         * @param bit the new bit.
         */
        inline void insert(const int index, const bool bit) noexcept {
            if (index == 0 && length == 0) {
                if (bit) {
                    words[0] = upperBitmask(1);
                } else {
                    words[0] = 0;
                }
                length++;
                return;
            }
            if (index == length) {
                //insert at the end of the bv
                const int bitPos = bitIndex(index);
                if (bitPos < wordSize && word(index) < (int)words.size()) {
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
                int wordIndex = word(index);
                //remember the last bit of the word (must be carried over to next word)
                bool carry = readBit(std::min(length - 1, wordIndex * wordSize + wordSize - 1));
                //shift all bits after relevant index back by one
                shiftBackFromIndex(index);
                //insert the new bit
                setBitTo(index, bit);
                //push carried bit through the remaining bv
                wordIndex++;
                while (wordIndex <= word(length - 1)) {
                    const bool newCarry = readBit(std::min(length - 1, wordIndex * wordSize + wordSize - 1));
                    //shift everything to the right
                    shiftBackFromIndex(wordIndex * wordSize);
                    setBitTo(wordIndex * wordSize, carry);//set first bit of the word to carry
                    carry = newCarry;
                    wordIndex++;
                }
                //insert last carried bit using the above base case.
                insert(length, carry);
            }
        }

        /**
         * Deletes the bit at the given index.
         * @param index the index that shall be deleted
         * @return the previous bit at that index
         */
        inline bool deleteIndex(const int index) noexcept {
            if (index >= length || length == 0)
                return false;
            const bool bit = readBit(index);
            if (word(index) == word(length - 1)) {
                //simply delete from the last word
                shiftLeftFromIndex(index);
            } else {
                //from the back to the front, move bits forward.
                int wordIndex = word(length - 1);//length > 0 is sure.
                //for the last word:
                bool carry = readBit(wordIndex * wordSize);//keep leftmost bit of the word
                words[wordIndex] <<= 1;//shift left by one bit
                wordIndex--;
                while (wordIndex > (int) word(index)) {
                    const bool newCarry = readBit(wordIndex * wordSize);
                    words[wordIndex] <<= 1;//shift left by one bit
                    setBitTo(wordIndex * wordSize + wordSize - 1, carry);
                    carry = newCarry;
                    wordIndex--;
                }
                //inside the relevant word, delete bit and insert carry as rightmost bit.
                shiftLeftFromIndex(index);
                setBitTo(std::min(length - 2, word(index) * wordSize + wordSize - 1), carry);
            }

            length--;
            //update capacity
            if (length + 2 * wBP < wordSize * (int)words.capacity()) //more than two words are unused
                shrink();

            return bit;
        }

        /**
         * Flips a bit.
         * @param index the index that shall be flipped.
         * @return the previous bit.
         */
        inline bool flipBit(const int index) noexcept {
            words[word(index)] ^= 1ULL << (wordSize - 1 - bitIndex(index));
            return !readBit(index);
        }

        /**
         * Reads the bit at index index
         * @param index the index
         * @return the bit at index index
         */
        inline bool readBit(const int index) const noexcept {
            return (words[word(index)] >> (wordSize - 1 - bitIndex(index))) & 1ULL;
        }

        /**
         * Executes select queries on bit vectors using a block-wise linear scan and popcount on 64-bit blocks.
         * @param j the number of the desired occurrence
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
         * Executes select queries on bit vectors using a block-wise linear scan and popcount on 64-bit blocks.
         * @param j the number of the desired occurrence
         * @return the index of the j-th zero in the bit vector
         */
        inline int selectZero(const int j) const noexcept {
            if (j == 0) return 0;
            int index = 0, count = 0;
            int relevantWordLength = std::min(wordSize, length);
            int nextWordCount = relevantWordLength - std::popcount(words[0]);
            while (count + nextWordCount < j) {
                //the desired index is not in the next word, we can skip it entirely
                count += nextWordCount;
                index += wordSize;
                relevantWordLength = std::min(wordSize, length - index);//for the last word, we need to ignore trailing zeros that dont belong to the bv
                nextWordCount = relevantWordLength - std::popcount(words[word(index)]);
            }
            //remaining select query on the word that contains the desired index.
            while (true) {
                if (!readBit(index)) count++;
                if (count == j) return index;
                index++;
            }
        }

        /**
         * Counts the number of ones in the bit vector using a block-wise scan.
         * @return the number of ones in the bv.
         */
        inline int popcount() const noexcept {
            return popcount(length);
        }

        /**
         * Returns the number of 1s in the bit vector up to the exclusive upper limit, i.e., performs a rank query.
         * @param upperLimit (exclusive!)
         * @return the number of ones.
         */
        inline int popcount(int upperLimit) const noexcept {
            int count = 0, index = 0;
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

        /**
         * Returns the length of the bit vector
         * @return the number of bits
         */
        inline int getLength() const noexcept {
            return length;
        }

        /**
         * Recomputes the excess information for this block.
         *
         * @return tuple (totalExcess, minExcess, minTimes)
         */
        inline std::tuple<int, int, int> recomputeExcesses() const noexcept {
            int excess = 0;
            int minExcess = inf;
            int numberOfMinima = 0;
            for (int i = 0; i < length; i++) {
                readBit(i) ? excess++ : excess--;
                if (excess < minExcess) {
                    minExcess = excess;
                    numberOfMinima = 1;
                } else if (excess == minExcess) {
                    numberOfMinima++;
                }
            }
            return std::make_tuple(excess, minExcess, numberOfMinima);
        }

        /**
         * Performs a forward search within this bit vector.
         *
         * @param i the start index within this bit vector
         * @param d the desired local excess
         * @return pair (achievedExcess, index)
         */
        inline std::pair<int, int> fwdBlock(int i, int d) const noexcept {
            int currentExcess = 0;
            int j = i + 1;
            for (; j < length; j++) {
                readBit(j) ? currentExcess++ : currentExcess--;
                if (currentExcess == d) return std::make_pair(d, j);
            }
            return std::make_pair(currentExcess, j);
        }

        /**
         * Performs a backward search in the bit vector.
         *
         * @param i the start index (searching to the left)
         * @param d the desired local excess
         * @return tuple (achievedExcess, index)
         */
        inline std::tuple<int, int> bwdBlock(int i, int d) const noexcept {
            int currentExcess = 0;
            for (int j = i; j >= 0; j--) {
                (!readBit(j)) ? currentExcess++ : currentExcess--;//negated condition!
                if (currentExcess == d) return std::make_pair(d, j);
            }
            return std::make_pair(currentExcess, -1);
        }

        /**
         * Computes the minimum local excess(i, k) in [i,j], i.e., k <= j.
         *
         * @param i the start index
         * @param j the end index (inclusive)
         * @return (the minimum local excess, the total local excess up to j)
         */
        inline std::pair<int, int> minBlock(int i, int j) const noexcept {
            int excess = 0;
            int minExcess = inf;
            for (int k = i; k <= j; k++) {
                readBit(k) ? excess++ : excess--;
                if (excess < minExcess) minExcess = excess;
            }
            return std::make_pair(minExcess, excess);
        }

        /**
         * Selects the t-th occurrence of the local excess theMinExcess within the bit vector.
         *
         * @param i the start index
         * @param j the end index (inclusive)
         * @param t the index of the occurrence
         * @param theMinExcess the desired excess
         * @return the position of the desired occurrence
         */
        inline int minSelectBlock(const int i, const int j, int t, const int theMinExcess) const noexcept {
            int excess = 0;
            for (int k = i; k <= j; k++) {
                readBit(k) ? excess++ : excess--;
                if (excess == theMinExcess) {
                    t--;
                    if (t == 0) {
                        return k;
                    }
                }
            }
            AssertMsg(false, "Occurrence of minExcess not found in block minSelect");
            return 0;
        }

        /**
         * Computes the number of occurrences of minimum as local excess in [i,j]
         *
         * @param i start index
         * @param j end index (inclusive)
         * @param minimum the desired minimum
         * @return (total excess, the number of occurrences)
         */
        inline std::pair<int, int> minCount(int i, int j, int minimum) const noexcept {
            int excess = 0;
            int count = 0;
            for (int k = i; k <= j; k++) {
                readBit(k) ? excess++ : excess--;
                if (excess == minimum) count++;
            }
            return std::make_pair(excess, count);
        }

    private:
        /**
         * Sets the bit at the desired index to bit.
         * @param index the index
         * @param bit the new bit value
         */
        inline void setBitTo(int index, bool bit) noexcept {
            AssertMsg(index < length, "Tried to change bit outside the valid range.");
            const int bitPos = bitIndex(index);
            words[word(index)] = (words[word(index)] & ~(1ULL << (wordSize - 1 - bitPos))) | ((uint64_t)bit << (wordSize - 1 - bitPos));
        }

        /**
         * Shifts all bits starting from (and including) index back by 1 step *within the word only*.
         * @param index the start index of the shift
         */
        inline void shiftBackFromIndex(int index) noexcept {
            const uint64_t bitmask = lowerBitmask(wordSize - bitIndex(index));//0..01..1
            uint64_t leftHalf = words[word(index)] & ~bitmask;//keep everything in front of bitIndex
            uint64_t rightHalf = words[word(index)] & bitmask;//keep everything starting from bitIndex
            words[word(index)] = leftHalf | (rightHalf >> 1);//shift right half by one, assemble result
        }

        /**
         * Shifts all bits behind index left by 1 step, overwriting the bit at index, *within the word only*.
         * @param index the index to be deleted
         */
        inline void shiftLeftFromIndex(int index) noexcept {
            const uint64_t leftBitMask = upperBitmask(bitIndex(index));
            const uint64_t rightBitMask = lowerBitmask(wordSize - bitIndex(index) - 1);
            uint64_t leftHalf = words[word(index)] & leftBitMask;//keep everything in front of bitIndex
            uint64_t rightHalf = words[word(index)] & rightBitMask;//keep everything behind bitIndex
            words[word(index)] = leftHalf | (rightHalf << 1);//shift right half by one, assemble result
        }

        /**
         * Shifts the bits starting from index forwards by delta bits, overwriting the delta bits
         * occurring directly in front of index.
         *
         * @param index the index, *always a multiple of wordSize*
         * @param delta the number of positions to be shifted by
         */
        inline void shiftForwardFromIndex(int index, int delta) noexcept {
            AssertMsg(index < wordSize * words.size(), "Shifted from invalid index.");
            int currentWord = word(length - 1);
            uint64_t bitmaskUpper = upperBitmask(delta);
            //get carry from last word
            uint64_t carry = words[currentWord] & bitmaskUpper;
            words[currentWord] <<= delta;
            currentWord--;
            while (currentWord >= word(index)) {
                //in each word, get new carry from the left, shift word left and add the old carry to the right side
                uint64_t newCarry = words[currentWord] & bitmaskUpper;
                words[currentWord] <<= delta;
                words[currentWord] |= (carry >> (wordSize - delta));
                carry = newCarry;
                currentWord--;
            }
            //the bitIndex(index) in the target word is always 0 as index is a multiple of wordSize
            //in words[word(index)], we shift everything left from index 0 inside the word
            //now, we need to overwrite the rightmost delta bits of words[word(index)] with the remaining carry.
            AssertMsg(word(index) >= 1, "Invalid left shift");
            uint64_t leftPart = words[word(index) - 1] & upperBitmask(wordSize - delta);//keep the left part of the word
            words[word(index) - 1] = leftPart | (carry >> (wordSize - delta));//add the remaining carry at the end
            length -= delta;
        }

        /**
         * Reserves more space
         */
        inline void enlarge() noexcept {
            words.reserve(words.capacity() + 1);
        }

        /**
         * Frees some space.
         */
        inline void shrink() noexcept {
            words.reserve(words.capacity() - 1);
        }

        /**
         * Removes any unused words at the end.
         */
        inline void shrinkToFit() noexcept {
            int numberOfWords = length / wordSize;
            if (length % wordSize != 0) numberOfWords++;
            words.resize(numberOfWords);
        }

        /**
         * Computes the word that contains the bit with the given index
         * @param index the index
         * @return the index of the relevant word within words
         */
        inline int word(int index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index / wordSize;
        }

        /**
         * Computes the index within its word of the bit with the given global index
         * @param index the desired global index
         * @return the index within the word only
         */
        inline int bitIndex(int index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index % wordSize;
        }

        /**
         * Creates a bitmask starting with numberOfOnes ones, followed by zeros.
         * @param numberOfOnes the number of ones
         * @return 1..10..0 with exactly numberOfOnes ones at the start.
         */
        inline uint64_t upperBitmask(uint64_t numberOfOnes) const noexcept {
            return ~(lowerBitmask(wordSize - numberOfOnes));

        }

        /**
         * Creates a bitmask ending with numberOfOnes ones.
         * @param numberOfOnes the number of ones
         * @return 0..01..1 with exactly numberOfOnes ones at the end.
         */
        inline uint64_t lowerBitmask(uint64_t numberOfOnes) const noexcept {
            if (numberOfOnes == 64) return (uint64_t)-1;
            return (1ULL << numberOfOnes) - 1;
        }

    public:
        /**
         * Frees the memory used for this bv.
         */
        inline void free() noexcept {
            std::vector<uint64_t>().swap(words);
            delete this;
        }

        /**
         * Calculates the number of bits needed for this bv.
         * @return the number of bits.
         */
        inline size_t getSize() const noexcept {
            return CHAR_BIT * (sizeof(length) + sizeof(words)) + words.size() * wordSize;
        }

        //some helper functions
        inline void printBitString(int offset = 0) const noexcept {
            std::cout << std::string(offset, ' ');
            for (int i = 0; i < length; i++) {
                std::cout << " " << readBit(i);
            }
            std::cout << "; length = " << length << std::endl;
        }

        inline void writeBitsToVector(std::vector<bool>* out) const noexcept {
            for (int i = 0; i < length; i++) {
                out->emplace_back(readBit(i));
            }
        }

        inline std::vector<bool> getBitString() const noexcept {
            std::vector<bool> result;
            writeBitsToVector(&result);
            return result;
        }

    public:
        std::vector<uint64_t> words;//all bits behind the first length bits must be 0!
        int length;
    };
}
