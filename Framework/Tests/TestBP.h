#include "catch.hpp"

#include "../BalancedParentheses/DynamicBP.h"

TEST_CASE("Small BP Test Instance", "[bp]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::NoProfiler> tree;

    SECTION("Initially empty tree") {
        std::vector<bool> expected = {1, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    tree.insertChild(0, 1, 0);
    tree.insertChild(0, 2, 0);
    tree.insertChild(0, 3, 0);
    tree.insertChild(0, 2, 1);
    tree.insertChild(0, 1, 0);
    tree.insertChild(0, 1, 2);
    tree.insertChild(1, 2, 1);

    SECTION("Basic inserts work") {
        std::vector<bool> expected = {1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    SECTION("Deleting nodes works") {
        tree.deleteNode(13);
        tree.deleteNode(2);

        std::vector<bool> expected = {1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    SECTION("Degree queries are correct") {
        REQUIRE(tree.degree(0) == 3);
        REQUIRE(tree.degree(1) == 2);
        REQUIRE(tree.degree(2) == 0);
        REQUIRE(tree.degree(4) == 1);
        REQUIRE(tree.degree(5) == 0);
        REQUIRE(tree.degree(9) == 1);
        REQUIRE(tree.degree(10) == 0);
        REQUIRE(tree.degree(13) == 0);
    }

    SECTION("Child queries are correct") {
        REQUIRE(tree.child(0, 1) == 1);
        REQUIRE(tree.child(0, 2) == 9);
        REQUIRE(tree.child(0, 3) == 13);
        REQUIRE(tree.child(1, 1) == 2);
        REQUIRE(tree.child(1, 2) == 4);
        REQUIRE(tree.child(4, 1) == 5);
        REQUIRE(tree.child(9, 1) == 10);
    }

    SECTION("Parent queries are correct") {
        REQUIRE(tree.parent(1) == 0);
        REQUIRE(tree.parent(2) == 1);
        REQUIRE(tree.parent(4) == 1);
        REQUIRE(tree.parent(5) == 4);
        REQUIRE(tree.parent(10) == 9);
        REQUIRE(tree.parent(9) == 0);
        REQUIRE(tree.parent(13) == 0);
    }

    SECTION("Subtree size queries are correct") {
        REQUIRE(tree.subtreeSize(0) == 8);
        REQUIRE(tree.subtreeSize(1) == 4);
        REQUIRE(tree.subtreeSize(2) == 1);
        REQUIRE(tree.subtreeSize(4) == 2);
        REQUIRE(tree.subtreeSize(5) == 1);
        REQUIRE(tree.subtreeSize(9) == 2);
        REQUIRE(tree.subtreeSize(10) == 1);
        REQUIRE(tree.subtreeSize(13) == 1);
    }
}