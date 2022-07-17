#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"
#include "../Helpers/Asserts.h"

namespace BalancedParentheses {
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
            bits.reserve(wBP);
        }

        /**
         * Creates a new InnerBitVector by splitting old.
         * Copies the blocks from old beginning at the middle to the new InnerBitVector, erases them from old.
         * @param old the existing bit vector.
         */
        InnerBitVector(InnerBitVector* old) :
                bits(0) {
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
            if (bits.size() + 2 * wBP < bits.capacity()) //more than two words are unused
                shrink();
            return bit;
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

        inline int getLength() const noexcept {
            return bits.size();
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
            for (size_t i = 0; i < bits.size(); i++) {
                bits[i] ? excess++ : excess--;
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
         * Performs a forward search within this bit vector using a simple linear scan.
         *
         * @param i the start index within this bit vector
         * @param d the desired local excess
         * @return pair (achievedExcess, index)
         */
        inline std::pair<int, int> fwdBlock(int i, int d) const noexcept {
            int currentExcess = 0;
            size_t j = i + 1;
            for (; j < bits.size(); j++) {
                bits[j] ? currentExcess++ : currentExcess--;
                if (currentExcess == d) return std::make_pair(d, j);
            }
            return std::make_pair(currentExcess, j);
        }

        /**
         * Performs a backward search in the bit vector using a simple linear scan.
         *
         * @param i the start index (searching to the left)
         * @param d the desired local excess
         * @return tuple (achievedExcess, index)
         */
        inline std::tuple<int, int> bwdBlock(int i, int d) const noexcept {
            int currentExcess = 0;
            for (int j = i; j >= 0; j--) {
                (!bits[j]) ? currentExcess++ : currentExcess--;//negated expression!
                if (currentExcess == d) return std::make_pair(d, j);
            }

            return std::make_pair(currentExcess, -1);
        }

        /**
         * Computes the minimum local excess(i, k) in [i,j], i.e., k <= j
         * using a simple linear scan.
         *
         * @param i the start index
         * @param j the end index (inclusive)
         * @return (the minimum local excess, the total local excess up to j)
         */
        inline std::pair<int, int> minBlock(int i, int j) const noexcept {
            int excess = 0;
            int minExcess = inf;
            for (int k = i; k <= j; k++) {
                bits[k] ? excess++ : excess--;
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
                bits[k] ? excess++ : excess--;
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
                bits[k] ? excess++ : excess--;
                if (excess == minimum) count++;
            }
            return std::make_pair(excess, count);
        }

        /**
         * Frees the memory needed for this bit vector.
         */
        inline void free() noexcept {
            std::vector<bool>().swap(bits);
            delete this;
        }

    private:
        /**
         * Reserves more space
         */
        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + wBP);
        }

        /**
         * Frees some space.
         */
        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - wBP);
        }

    public:
        //some helper functions
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

        inline size_t getSize() const noexcept {
            return CHAR_BIT * sizeof(bits) + bits.size();
        }

        std::vector<bool> bits;
    };
}
