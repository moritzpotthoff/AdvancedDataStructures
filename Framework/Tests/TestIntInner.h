#pragma once

#include "catch.hpp"

#include <math.h>

#include "../BitVector/DynamicBitVector.h"
#include "../BitVector/InnerBitVector.h"
#include "../BitVector/InnerBitVectorByInt.h"
#include "../Helpers/BitVectorProfiler.h"

TEST_CASE("Simple test for inner bv with int", "[inner][simple]") {
    BitVector::InnerBitVectorByInt bv;

    const int numberOfBits = 1000;
    std::vector<bool> expected = {};
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0);
        bv.insert(i, bit);
        expected.insert(expected.begin() + i, bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        bv.deleteIndex(0);
    }

    std::vector<bool> empty = {};
    REQUIRE(bv.getBitString() == empty);
}

TEST_CASE("Simple insert for inner bv with int", "[inner][insert]") {
    BitVector::InnerBitVectorByInt bv;

    const int numberOfBits = 10;
    std::vector<bool> expected = {};
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0);
        bv.insert(i, bit);
        expected.insert(expected.begin() + i, bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 4 == 0);
        bv.insert(0, bit);
        expected.insert(expected.begin(), bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 4 == 0);
        bv.insert(12, bit);
        expected.insert(expected.begin() + 12, bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        bv.deleteIndex(numberOfBits + i);
        expected.erase(expected.begin() + numberOfBits + i);
        REQUIRE(bv.getBitString() == expected);
    }
}