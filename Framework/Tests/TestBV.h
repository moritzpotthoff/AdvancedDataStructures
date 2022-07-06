#pragma once

#include "catch.hpp"

#include <math.h>

#include "../BitVector/DynamicBitVector.h"
#include "../BitVector/InnerBitVector.h"
#include "../BitVector/InnerBitVectorByInt.h"
#include "../Helpers/BitVectorProfiler.h"

TEST_CASE("Small simple BV Test Instance", "[bv][small]") {
    //BitVector::DynamicBitVector<BitVector::NoProfiler> bv;
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv;

    std::vector<bool> expected = {};
    REQUIRE(bv.getBitString() == expected);
    const int numberOfBits = 100;
    const int numberOfOnes = numberOfBits / 2;

    SECTION("Works with only 0") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(false);
            bv.insertBit(i, false);
            REQUIRE(bv.getBitString() == expected);
        }

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

TEST_CASE("Large simple BV Test Instance", "[bv][simple][large]") {
    //BitVector::DynamicBitVector<BitVector::NoProfiler> bv;
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv;

    std::vector<bool> expected = {};
    std::vector<bool> expectedWithToDelete = {};
    REQUIRE(bv.getBitString() == expected);
    const int numberOfBits = 100000;
    const int deleteBits = numberOfBits / 10;
    const int numberOfOnes = numberOfBits / 2;

    for (int i = 0; i < deleteBits; i++) {
        bv.insertBit(i, i % 7 == 3);
        expectedWithToDelete.push_back(i % 7 == 3);
    }

    SECTION("Works with only 0") {
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(false);
            expectedWithToDelete.push_back(false);
            bv.insertBit(i + deleteBits, false);
            REQUIRE(bv.getBitString() == expectedWithToDelete);
        }

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
            expectedWithToDelete.erase(expectedWithToDelete.begin() + i);
            REQUIRE(bv.getBitString() == expectedWithToDelete);
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
            expectedWithToDelete.push_back(true);
            bv.insertBit(i + deleteBits, true);
            REQUIRE(bv.getBitString() == expectedWithToDelete);
        }

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
            expectedWithToDelete.erase(expectedWithToDelete.begin() + i);
            REQUIRE(bv.getBitString() == expectedWithToDelete);
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

TEST_CASE("BV initialized creation test", "[bv][create][simple]") {
    //BitVector::DynamicBitVector<BitVector::NoProfiler> bvNormal;
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bvNormal;

    const int numberOfBits = 123;
    std::vector<bool> expected = {};
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0);
        bvNormal.insertBit(i, bit);
        expected.push_back(bit);
    }
    //BitVector::DynamicBitVector<BitVector::NoProfiler> bvCreate(expected);
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bvCreate(expected);
    REQUIRE(bvNormal.getBitString() == expected);
    REQUIRE(bvCreate.getBitString() == expected);
}

/*
TEST_CASE("BV runtime test", "[bv][time]") {
    //BitVector::DynamicBitVector<BitVector::NoProfiler> bv;
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv;

    const int numberOfBits = 1000000;
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0);
        bv.insertBit(0, bit);
    }
    for (int i = 0; i < numberOfBits; i++) {
        bv.deleteBit(0);
    }

    std::vector<bool> expected = {};
    REQUIRE(bv.getBitString() == expected);
    bv.profiler.print();
}
 */

TEST_CASE("Large BV test instance with directly created BV.", "[bv][large][create]") {

    std::vector<bool> expected = {};
    const int numberOfBits = 100000;
    const int deleteBits = numberOfBits / 10;
    const int numberOfOnes = numberOfBits / 2;

    SECTION("Works with only 0") {
        for (int i = 0; i < deleteBits; i++) {
            expected.push_back(i % 7 == 3);
        }
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(false);
        }
        //BitVector::DynamicBitVector<BitVector::NoProfiler> bv(expected);
        BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv(expected);

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
            expected.erase(expected.begin() + i);
            //bv.validate();
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
        for (int i = 0; i < deleteBits; i++) {
            expected.push_back(i % 7 == 3);
        }
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(true);
        }
        //BitVector::DynamicBitVector<BitVector::NoProfiler> bv(expected);
        BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv(expected);


        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
            expected.erase(expected.begin() + i);
            //bv.validate();
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
        for (int i = 0; i < deleteBits; i++) {
            expected.push_back(i % 7 == 3);
        }
        for (int i = 0; i < numberOfBits; i++) {
            expected.push_back(i % 2 == 0);
        }
        //BitVector::DynamicBitVector<BitVector::NoProfiler> bv(expected);
        BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv(expected);

        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }
        for (int i = numberOfBits / 3; i < 2 * numberOfBits / 3; i++) {
            bv.flipBit(i);
            //bv.validate();
        }

        for (int i = deleteBits - 1; i >= 0; i--) {
            bv.deleteBit(i);
            expected.erase(expected.begin() + i);
            //bv.validate();
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

TEST_CASE("Simple inner bv test", "[bv][withInner][simple]") {
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv;

    const int numberOfBits = 100000;
    std::vector<bool> expected = {};
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0 || i % 3 == 0);
        bv.insertBit(0, bit);
        expected.insert(expected.begin(), bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        bv.deleteBit(0);
        expected.erase(expected.begin());
        REQUIRE(bv.getBitString() == expected);
    }

    std::vector<bool> empty = {};
    REQUIRE(bv.getBitString() == empty);
    bv.profiler.print();
}