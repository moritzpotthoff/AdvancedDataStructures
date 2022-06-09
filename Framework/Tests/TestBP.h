#include "catch.hpp"

#include "../BalancedParentheses/DynamicBP.h"

TEST_CASE("vectors can be sized and resized", "[bp]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::NoProfiler> tree;
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
    SECTION("Deleting the nodes works") {
        tree.deleteNode(13);
        tree.deleteNode(2);

        std::vector<bool> expected = {1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};
        REQUIRE(tree.getBitString() == expected);
    }
}