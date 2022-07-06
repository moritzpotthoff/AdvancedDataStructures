#pragma once

#include "catch.hpp"

#include <math.h>

#include "../BitVector/DynamicBitVector.h"
#include "../BitVector/InnerBitVector.h"
#include "../BitVector/InnerBitVectorByInt.h"
#include "../Helpers/BitVectorProfiler.h"

TEST_CASE("Simple insert for inner bv with int", "[inner][insert]") {
    BitVector::InnerBitVectorByInt bv;

    const int numberOfBits = 100;
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

TEST_CASE("Tests the internal bitmask functions.", "[inner][bitmask]") {
    BitVector::InnerBitVectorByInt bv;

    uint64_t lower0 = bv.lowerBitmask(0);
    REQUIRE(lower0 == 0ULL);
    uint64_t lower1 = bv.lowerBitmask(1);
    REQUIRE(lower1 == 0b0000000000000000000000000000000000000000000000000000000000000001);
    uint64_t lower10 = bv.lowerBitmask(10);
    REQUIRE(lower10 == 0b0000000000000000000000000000000000000000000000000000001111111111);
    uint64_t lower20 = bv.lowerBitmask(20);
    REQUIRE(lower20 == 0b0000000000000000000000000000000000000000000011111111111111111111);
    uint64_t lower63 = bv.lowerBitmask(63);
    REQUIRE(lower63 == 0b0111111111111111111111111111111111111111111111111111111111111111);
    uint64_t lower64 = bv.lowerBitmask(64);
    REQUIRE(lower64 == 0b1111111111111111111111111111111111111111111111111111111111111111);

    uint64_t upper0 = bv.upperBitmask(0);
    REQUIRE(upper0 == 0ULL);
    uint64_t upper1 = bv.upperBitmask(1);
    REQUIRE(upper1 == 0b1000000000000000000000000000000000000000000000000000000000000000);
    uint64_t upper10 = bv.upperBitmask(10);
    REQUIRE(upper10 == 0b1111111111000000000000000000000000000000000000000000000000000000);
    uint64_t upper20 = bv.upperBitmask(20);
    REQUIRE(upper20 == 0b1111111111111111111100000000000000000000000000000000000000000000);
    uint64_t upper63 = bv.upperBitmask(63);
    REQUIRE(upper63 == 0b1111111111111111111111111111111111111111111111111111111111111110);
    uint64_t upper64 = bv.upperBitmask(64);
    REQUIRE(upper64 == 0b1111111111111111111111111111111111111111111111111111111111111111);
}

TEST_CASE("Tests the internal shift back function.", "[inner][shiftBack]") {
    BitVector::InnerBitVectorByInt bv;
    bv.length = 100;
    bv.words[0] = bv.lowerBitmask(20);
    bv.words.emplace_back(bv.lowerBitmask(10));

    SECTION("Test index 0") {
        bv.shiftBackFromIndex(0);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == bv.lowerBitmask(19));
    }
    SECTION("Test index 1") {
        bv.shiftBackFromIndex(1);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == bv.lowerBitmask(19));
    }
    SECTION("Test index 43") {
        bv.shiftBackFromIndex(43);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == bv.lowerBitmask(19));
    }
    SECTION("Test index 44") {
        bv.shiftBackFromIndex(44);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == bv.lowerBitmask(19));
    }
    SECTION("Test index 45") {
        bv.shiftBackFromIndex(45);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000010111111111111111111);
    }
    SECTION("Test index 63") {
        bv.shiftBackFromIndex(63);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000011111111111111111110);
    }
}

TEST_CASE("Tests the internal shift left function.", "[inner][shiftLeft]") {
    BitVector::InnerBitVectorByInt bv;
    bv.length = 100;
    bv.words[0] = bv.lowerBitmask(20);
    bv.words.emplace_back(bv.lowerBitmask(10));

    SECTION("Test index 0") {
        bv.shiftLeftFromIndex(0);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000111111111111111111110);
    }
    SECTION("Test index 1") {
        bv.shiftLeftFromIndex(1);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000111111111111111111110);
    }
    SECTION("Test index 43 ") {
        bv.shiftLeftFromIndex(43);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000111111111111111111110);
    }
    SECTION("Test index 44") {
        bv.shiftLeftFromIndex(44);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000011111111111111111110);
    }
    SECTION("Test index 45") {
        bv.shiftLeftFromIndex(45);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000011111111111111111110);
    }
    SECTION("Test index 63") {
        bv.shiftLeftFromIndex(63);
        REQUIRE(bv.words[1] == bv.lowerBitmask(10));
        REQUIRE(bv.words[0] == 0b0000000000000000000000000000000000000000000011111111111111111110);
    }
}

TEST_CASE("Tests the internal shift forward by delta function.", "[inner][shiftForwardDelta]") {
    BitVector::InnerBitVectorByInt bv;
    bv.length = 180;
    bv.words[0] = 0;
    bv.words.emplace_back(bv.upperBitmask(20));
    bv.words.emplace_back(bv.upperBitmask(20));

    SECTION("Test first word") {
        bv.shiftForwardFromIndex(64, 20);
        REQUIRE(bv.words[0] == bv.lowerBitmask(20));
        REQUIRE(bv.words[1] == bv.lowerBitmask(20));
        REQUIRE(bv.words[2] == 0);
    }

    SECTION("Test second word") {
        bv.shiftForwardFromIndex(128, 20);
        REQUIRE(bv.words[0] == 0);
        REQUIRE(bv.words[1] == (bv.upperBitmask(20) | bv.lowerBitmask(20)));
        REQUIRE(bv.words[2] == 0);
    }
}

TEST_CASE("Tests the internal setBitTo function.", "[inner][setBitTo]") {
    BitVector::InnerBitVectorByInt bv;
    const size_t numberOfBits = 1000;
    bv.length = numberOfBits;
    for (size_t word = 0; word < numberOfBits / 64; word++) {
        bv.words.emplace_back(0);
    }

    std::vector<bool> expected = {};
    for (size_t i = 0; i < numberOfBits; i++) {
        expected.push_back(false);
    }
    for (size_t i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0);
        bv.setBitTo(i, bit);
        expected[i] = bit;
        REQUIRE(bv.getBitString() == expected);
    }
    for (size_t i = 0; i < numberOfBits; i++) {
        bv.setBitTo(i, false);
        expected[i] = false;
        REQUIRE(bv.getBitString() == expected);
    }
    for (size_t i = 0; i < numberOfBits; i++) {
        bv.setBitTo(i, true);
        expected[i] = true;
        REQUIRE(bv.getBitString() == expected);
    }
}

TEST_CASE("Simple test for inner bv with int", "[inner][simple]") {
    BitVector::InnerBitVectorByInt bv;

    const int numberOfBits = 100000;
    std::vector<bool> expected = {};
    for (int i = 0; i < numberOfBits; i++) {
        const bool bit = (i % 7 == 0 || i % 3 == 0);
        bv.insert(i, bit);
        expected.insert(expected.begin() + i, bit);
        REQUIRE(bv.getBitString() == expected);
    }
    for (int i = 0; i < numberOfBits; i++) {
        bv.deleteIndex(0);
        expected.erase(expected.begin());
        REQUIRE(bv.getBitString() == expected);
    }

    std::vector<bool> empty = {};
    REQUIRE(bv.getBitString() == empty);
}