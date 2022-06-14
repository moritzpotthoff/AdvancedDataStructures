#pragma once

#include <cstddef>
#include <bit>

#include "Definitions.h"
#include "../Helpers/Asserts.h"

namespace BalancedParentheses {
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
            //TODO check if this needs index - 1.
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
         *
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
         * Performs a forward search within this block.
         *
         * @param i the start index within this bit vector
         * @param d the desired local excess
         * @return pair (achievedExcess, index)
         */
        inline std::pair<int, int> fwdBlock(int i, int d) const noexcept {
            //TODO use actual block variant
            int currentExcess = 0;
            size_t j = i + 1;
            for (; j < bits.size(); j++) {
                bits[j] ? currentExcess++ : currentExcess--;
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
            //TODO use actual block variant
            int currentExcess = 0;
            for (int j = i; j >= 0; j--) {
                (!bits[j]) ? currentExcess++ : currentExcess--;//negated expression!
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

        inline void free() noexcept {
            std::vector<bool>().swap(bits);
            delete this;
        }

    private:

        inline void enlarge() noexcept {
            bits.reserve(bits.capacity() + w);
        }

        inline void shrink() noexcept {
            bits.reserve(bits.capacity() - w);
        }

    public:
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
