#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"

#define wordSize 64 //64 bits per uint64_t

namespace BalancedParentheses {
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
         * @param index
         * @param other
         * @param otherSize
         */
        inline void insertBitVector(const int index, InnerBitVectorByInt* other, int otherSize) noexcept {
            AssertMsg(index == 0 || index == length, "Called insertBitVector with index not start or end.");
            const int oldLength = length;
            if (index == 0) {
                //insert other at start
                int otherWords = other->length / wordSize;
                if (other->length % wordSize != 0) otherWords++;
                words.insert(words.begin(), other->words.begin(), other->words.begin() + otherWords);
                length += otherWords * wordSize;//temporarily adjust length for the following operations
                if (other->length % wordSize != 0) {
                    //remove the gap between the two
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
                    //remove the gap between the two
                    const int delta = wordSize - (oldLength % wordSize);
                    shiftForwardFromIndex(oldNumberOfWords * wordSize, delta);
                }
            }
            length = oldLength + otherSize;//actually set correct length
            //TODO delete other
        }

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
         * @param index
         * @return
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
            //update capacity//TODO reinsert
            //if (length + 2 * w < wordSize * (int)words.capacity()) //more than two words are unused
            //    shrink();

            return bit;
        }

        /**
         * Flips the bit and returns the previous bit.
         * @param index
         * @return
         */
        inline bool flipBit(const int index) noexcept {
            words[word(index)] ^= 1ULL << (wordSize - 1 - bitIndex(index));
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

        inline int popcount() const noexcept {
            return popcount(length);
        }

        /**
         * Returns the number of 1s in the bit vector up to the exclusive upper limit
         * @param upperLimit (exclusive!)
         * @return
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
         * Performs a forward search within this block.
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
         * Performs a backward search in the bit vector
         *
         * @param i the start index (searching to the left)
         * @param d the desired local excess
         * @return tuple (achievedExcess, index)
         */
        inline std::tuple<int, int> bwdBlock(int i, int d) const noexcept {
            int currentExcess = 0;
            for (int j = i; j >= 0; j--) {
                (!readBit(j)) ? currentExcess++ : currentExcess--;//negated expression!
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
         * @param index
         * @param bit
         */
        inline void setBitTo(int index, bool bit) noexcept {
            AssertMsg(index < length, "Tried to change bit outside the valid range.");
            const int bitPos = bitIndex(index);
            words[word(index)] = (words[word(index)] & ~(1ULL << (wordSize - 1 - bitPos))) | ((uint64_t)bit << (wordSize - 1 - bitPos));
        }

        /**
         * shifts all bits starting from (and including) index back by 1 step *within the word only*.
         * @param index
         */
        inline void shiftBackFromIndex(int index) noexcept {
            const uint64_t bitmask = lowerBitmask(wordSize - bitIndex(index));//0..01..1
            uint64_t leftHalf = words[word(index)] & ~bitmask;//keep everything in front of bitIndex
            uint64_t rightHalf = words[word(index)] & bitmask;//keep everything starting from bitIndex
            words[word(index)] = leftHalf | (rightHalf >> 1);//shift right half by one, assemble result
        }

        /**
         * Shifts all bits behind index left by 1 step, overwriting the bit at index, *within the word only*.
         * @param index
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
         * occuring directly in front of index.
         *
         * @param index *always a multiple of wordSize*!!!
         * @param delta
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

        inline void enlarge() noexcept {
            words.reserve(words.capacity() + 1);
        }

        inline void shrink() noexcept {
            words.reserve(words.capacity() - 1);
        }

        //make sure there are no unused words at the end
        inline void shrinkToFit() noexcept {
            int numberOfWords = length / wordSize;
            if (length % wordSize != 0) numberOfWords++;
            words.resize(numberOfWords);
        }

        inline int word(int index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index / wordSize;
        }

        inline int bitIndex(int index) const noexcept {
            AssertMsg(index <= length, "Invalid index for inner bit vector.");
            return index % wordSize;
        }

        inline uint64_t upperBitmask(uint64_t numberOfOnes) const noexcept {
            return ~(lowerBitmask(wordSize - numberOfOnes));//1..10..0 with exactly numberOfOnes ones at the start.

        }

        inline uint64_t lowerBitmask(uint64_t numberOfOnes) const noexcept {
            if (numberOfOnes == 64) return (uint64_t)-1;
            return (1ULL << numberOfOnes) - 1;//0..01..1 with exactly numberOfOnes ones at the end.
        }

    public:
        inline void free() noexcept {
            std::vector<uint64_t>().swap(words);
            delete this;
        }

        inline size_t getSize() const noexcept {
            return CHAR_BIT * (sizeof(length) + sizeof(words)) + words.size() * wordSize;
        }

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
        std::vector<uint64_t> words;//all bits behind the first length bits must be 0! size must be maintained correctly.
        int length;
    };
}
