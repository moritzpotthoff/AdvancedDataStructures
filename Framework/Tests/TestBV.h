#pragma once

#include "catch.hpp"

#include <math.h>

#include "../BitVector/DynamicBitVector.h"
#include "../Helpers/BitVectorProfiler.h"

TEST_CASE("Small simple BV Test Instance", "[bv][small]") {
    BitVector::DynamicBitVector<BitVector::NoProfiler> bv;

    std::vector<bool> expected = {};
    REQUIRE(bv.getBitString() == expected);
    const int numberOfBits = 100;
    const int numberOfOnes = numberOfBits / 2;

    SECTION("Works with only 0") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(false);
            bv.insertBit(i, false);
        }
        REQUIRE(bv.getBitString() == expected);

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = 0; i < numberOfBits; i++) {
            REQUIRE(bv.access(i) == false);
            REQUIRE(bv.rank(true, i) == 0);
            REQUIRE(bv.rank(false, i) == i);
            REQUIRE(bv.select(false, i + 1) == i);
        }
    }

    SECTION("Works with only 1") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(true);
            bv.insertBit(i, true);
        }
        REQUIRE(bv.getBitString() == expected);

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = 0; i < numberOfBits; i++) {
            REQUIRE(bv.access(i) == true);
            REQUIRE(bv.rank(true, i) == i);
            REQUIRE(bv.rank(false, i) == 0);
            REQUIRE(bv.select(true, i + 1) == i);
        }
    }

    SECTION("Works with 01") {
        for (int i = 0; i < numberOfOnes; i++) {
            expected.push_back(true);
            expected.push_back(false);
            bv.insertBit(2 * i, true);
            bv.insertBit(2 * i + 1, false);
        }
        REQUIRE(bv.getBitString() == expected);

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = 0; i < numberOfBits; i++) {
            bool bit = i % 2 == 0;
            REQUIRE(bv.access(i) == bit);
            REQUIRE(bv.rank(true, i) == (i + 1) / 2);
            REQUIRE(bv.rank(false, i) == i / 2);
        }
        for (int i = 1; i <= numberOfOnes; i++) {
            REQUIRE(bv.select(true, i) == 2 * (i - 1));
            REQUIRE(bv.select(false, i) == 2 * (i - 1) + 1);
        }
    }
}

TEST_CASE("Large simple BV Test Instance", "[bv][large]") {
    BitVector::DynamicBitVector<BitVector::NoProfiler> bv;

    std::vector<bool> expected = {};
    REQUIRE(bv.getBitString() == expected);
    const int numberOfBits = 1000000;
    const int deleteBits = numberOfBits / 10;
    const int numberOfOnes = numberOfBits / 2;

    for (int i = 0; i < deleteBits; i++) {
        bv.insertBit(i, i % 7 == 3);
    }

    SECTION("Works with only 0") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(false);
            bv.insertBit(i + deleteBits, false);
        }

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
        }

        REQUIRE(bv.getBitString() == expected);

        for (int i = 0; i < numberOfBits; i++) {
            REQUIRE(bv.access(i) == false);
            REQUIRE(bv.rank(true, i) == 0);
            REQUIRE(bv.rank(false, i) == i);
            REQUIRE(bv.select(false, i + 1) == i);
        }
    }

    SECTION("Works with only 1") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(true);
            bv.insertBit(i + deleteBits, true);
        }

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
        }

        REQUIRE(bv.getBitString() == expected);

        for (int i = 0; i < numberOfBits; i++) {
            REQUIRE(bv.access(i) == true);
            REQUIRE(bv.rank(true, i) == i);
            REQUIRE(bv.rank(false, i) == 0);
            REQUIRE(bv.select(true, i + 1) == i);
        }
    }

    SECTION("Works with 01") {
        for (int i = 0; i < numberOfOnes; i++) {
            expected.push_back(true);
            expected.push_back(false);
            bv.insertBit(2 * i + deleteBits, true);
            bv.insertBit(2 * i + 1 + deleteBits, false);
        }

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
        }

        REQUIRE(bv.getBitString() == expected);

        for (int i = 0; i < numberOfBits; i++) {
            bool bit = i % 2 == 0;
            REQUIRE(bv.access(i) == bit);
            REQUIRE(bv.rank(true, i) == (i + 1) / 2);
            REQUIRE(bv.rank(false, i) == i / 2);
        }
        for (int i = 1; i <= numberOfOnes; i++) {
            REQUIRE(bv.select(true, i) == 2 * (i - 1));
            REQUIRE(bv.select(false, i) == 2 * (i - 1) + 1);
        }
    }
}
