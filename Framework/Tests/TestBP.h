#pragma once

#include "catch.hpp"

#include <math.h>

#include "../BalancedParentheses/DynamicBP.h"

TEST_CASE("Small BP Test Instance", "[bp][small]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::BasicProfiler, BalancedParentheses::InnerBitVectorByInt> tree;

    SECTION("Initially empty tree") {
        std::vector<bool> expected = {1, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    std::cout << "Creating miniature test tree." << std::endl;

    tree.insertChild(0, 1, 0);
    tree.insertChild(0, 2, 0);
    tree.insertChild(0, 3, 0);
    tree.insertChild(0, 2, 1);
    tree.insertChild(0, 1, 0);
    tree.insertChild(0, 1, 2);
    tree.insertChild(1, 2, 1);

    std::vector<bool> expected = {1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0};
    REQUIRE(tree.getBitString() == expected);

    REQUIRE(tree.degree(0) == 3);
    REQUIRE(tree.degree(1) == 2);
    REQUIRE(tree.degree(2) == 0);
    REQUIRE(tree.degree(4) == 1);
    REQUIRE(tree.degree(5) == 0);
    REQUIRE(tree.degree(9) == 1);
    REQUIRE(tree.degree(10) == 0);
    REQUIRE(tree.degree(13) == 0);

    REQUIRE(tree.child(0, 1) == 1);
    REQUIRE(tree.child(0, 2) == 9);
    REQUIRE(tree.child(0, 3) == 13);
    REQUIRE(tree.child(1, 1) == 2);
    REQUIRE(tree.child(1, 2) == 4);
    REQUIRE(tree.child(4, 1) == 5);
    REQUIRE(tree.child(9, 1) == 10);

    REQUIRE(tree.parent(1) == 0);
    REQUIRE(tree.parent(2) == 1);
    REQUIRE(tree.parent(4) == 1);
    REQUIRE(tree.parent(5) == 4);
    REQUIRE(tree.parent(10) == 9);
    REQUIRE(tree.parent(9) == 0);
    REQUIRE(tree.parent(13) == 0);

    REQUIRE(tree.subtreeSize(0) == 8);
    REQUIRE(tree.subtreeSize(1) == 4);
    REQUIRE(tree.subtreeSize(2) == 1);
    REQUIRE(tree.subtreeSize(4) == 2);
    REQUIRE(tree.subtreeSize(5) == 1);
    REQUIRE(tree.subtreeSize(9) == 2);
    REQUIRE(tree.subtreeSize(10) == 1);
    REQUIRE(tree.subtreeSize(13) == 1);

    SECTION("Deleting nodes works") {
        tree.deleteNode(13);
        tree.deleteNode(2);

        std::vector<bool> expected = {1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0};
        REQUIRE(tree.getBitString() == expected);

        REQUIRE(tree.degree(0) == 2);
        REQUIRE(tree.degree(1) == 1);
        REQUIRE(tree.degree(2) == 1);
        REQUIRE(tree.degree(3) == 0);
        REQUIRE(tree.degree(7) == 1);
        REQUIRE(tree.degree(8) == 0);

        REQUIRE(tree.child(0, 1) == 1);
        REQUIRE(tree.child(0, 2) == 7);
        REQUIRE(tree.child(1, 1) == 2);
        REQUIRE(tree.child(2, 1) == 3);
        REQUIRE(tree.child(7, 1) == 8);

        REQUIRE(tree.parent(1) == 0);
        REQUIRE(tree.parent(2) == 1);
        REQUIRE(tree.parent(3) == 2);
        REQUIRE(tree.parent(7) == 0);
        REQUIRE(tree.parent(8) == 7);

        REQUIRE(tree.subtreeSize(0) == 6);
        REQUIRE(tree.subtreeSize(1) == 3);
        REQUIRE(tree.subtreeSize(2) == 2);
        REQUIRE(tree.subtreeSize(3) == 1);
        REQUIRE(tree.subtreeSize(7) == 2);
        REQUIRE(tree.subtreeSize(8) == 1);
    }

    tree.profiler.print();
}

TEST_CASE("Large BP Test Instance", "[bp][large][flat]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::BasicProfiler, BalancedParentheses::InnerBitVectorByInt> tree;

    SECTION("Initially empty tree") {
        std::vector<bool> expected = {1, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    const int numberOfChildren = 100000;
    std::cout << "Creating flat tree with " << numberOfChildren << " nodes in the first level." << std::endl;
    //insert correct number of nodes for this level, getting the correct number of children themselves
    for (int node = 1; node <= numberOfChildren; node++) {
        tree.insertChild(0, node, 0);
    }


    REQUIRE(tree.subtreeSize(0) == numberOfChildren + 1);
    REQUIRE(tree.degree(0) == numberOfChildren);
    for (int i = 0; i < numberOfChildren; i++) {
        const int childStartIndex = 2 * i + 1;
        REQUIRE(tree.child(0, i + 1) == childStartIndex);
        REQUIRE(tree.subtreeSize(childStartIndex) == 1);
        REQUIRE(tree.degree(childStartIndex) == 0);
        REQUIRE(tree.parent(childStartIndex) == 0);
    }

    const int innerNodeDegree = 200;
    std::cout << "Inserting " << numberOfChildren / innerNodeDegree << " children in the middle." << std::endl;
    for (int i = 0; i < numberOfChildren / innerNodeDegree; i++) {
        tree.insertChild(0, i + 1, innerNodeDegree);
    }

    REQUIRE(tree.subtreeSize(0) == numberOfChildren + numberOfChildren / innerNodeDegree + 1);
    REQUIRE(tree.degree(0) == numberOfChildren / innerNodeDegree);
    for (int innerNode = 0; innerNode < numberOfChildren / innerNodeDegree; innerNode++) {
        const int innerNodeStartPos = (innerNodeDegree * 2 + 2) * innerNode + 1;
        //tree.printBitString();
        REQUIRE(tree.child(0, innerNode + 1) == innerNodeStartPos);
        REQUIRE(tree.degree(innerNodeStartPos) == innerNodeDegree);
        REQUIRE(tree.subtreeSize(innerNodeStartPos) == innerNodeDegree + 1);
        for (int child = 0; child < innerNodeDegree; child++) {
            const int childStartPos = tree.child(innerNodeStartPos, child + 1);
            REQUIRE(tree.degree(childStartPos) == 0);
            REQUIRE(tree.parent(childStartPos) == innerNodeStartPos);
            REQUIRE(tree.subtreeSize(childStartPos) == 1);
        }
    }

    for (int i = 0; i < numberOfChildren + numberOfChildren / innerNodeDegree; i++) {
        tree.deleteNode(1);
    }
    REQUIRE(tree.degree(0) == 0);
    REQUIRE(tree.subtreeSize(0) == 1);
    std::vector<bool> expected = {1, 0};
    REQUIRE(tree.getBitString() == expected);

    tree.profiler.print();
}

TEST_CASE("Binary Tree Test", "[bp][large][binary]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::BasicProfiler, BalancedParentheses::InnerBitVectorByInt> tree;

    //create complete binary tree
    const int numberOfLevels = 16;
    const int numberOfNodes = pow(2, numberOfLevels + 1) - 1;
    std::cout << "Creating full binary tree with " << numberOfNodes << " nodes." << std::endl;
    for (int level = numberOfLevels; level > 0; level--) {
        //no children for leaves
        const int numberOfChildren = (level == numberOfLevels) ? 0 : 2;
        //insert correct number of nodes for this level, getting the correct number of children themselves
        for (int node = 1; node <= pow(2, level); node++) {
            tree.insertChild(0, node, numberOfChildren);
        }
    }

    for (int i = 0; i < numberOfLevels; i++) {
        REQUIRE(tree.degree(i) == 2);
        REQUIRE(tree.subtreeSize(i) == pow(2, numberOfLevels + 1 - i) - 1);
    }
    REQUIRE(tree.degree(numberOfLevels) == 0);

    SECTION("Delete layer 1 (below root)") {
        //delete layer 1 (below root)
        tree.deleteNode(tree.child(0, 2));
        tree.deleteNode(tree.child(0, 1));
        REQUIRE(tree.degree(0) == 4);//now has all nodes from next level
        REQUIRE(tree.subtreeSize(0) == pow(2, numberOfLevels + 1) - 3);//two nodes missing
        REQUIRE(tree.subtreeSize(1) == pow(2, numberOfLevels - 1) - 1);//same number of nodes from level 2 onwards
        REQUIRE(tree.subtreeSize(tree.child(0, 2)) == pow(2, numberOfLevels - 1) - 1);//same number of nodes from level 2 onwards
        REQUIRE(tree.subtreeSize(tree.child(0, 3)) == pow(2, numberOfLevels - 1) - 1);//same number of nodes from level 2 onwards
        REQUIRE(tree.subtreeSize(tree.child(0, 4)) == pow(2, numberOfLevels - 1) - 1);//same number of nodes from level 2 onwards
        REQUIRE(tree.parent(1) == 0);
        REQUIRE(tree.parent(tree.child(0, 2)) == 0);
        REQUIRE(tree.parent(tree.child(0, 3)) == 0);
        REQUIRE(tree.parent(tree.child(0, 4)) == 0);
    }

    SECTION("Delete all nodes except root") {
        for (int level = numberOfLevels; level > 0; level--) {
            for (int node = 1; node <= pow(2, level); node++) {
                tree.deleteNode(1);//delete the current first child of root.
            }
        }

        REQUIRE(tree.degree(0) == 0);
        REQUIRE(tree.subtreeSize(0) == 1);
        std::vector<bool> expected = {1, 0};
        REQUIRE(tree.getBitString() == expected);
    }

    tree.profiler.print();
}

TEST_CASE("Linear Tree Test", "[bp][large][linear]") {
    BalancedParentheses::DynamicBP<BalancedParentheses::BasicProfiler, BalancedParentheses::InnerBitVectorByInt> tree;

    const int numberOfLevels = 100000;
    std::cout << "Creating linear tree with " << numberOfLevels << " levels." << std::endl;
    for (int level = 0; level < numberOfLevels; level++) {
        tree.insertChild(level, 1, 0);
    }

    for (int i = 0; i < numberOfLevels; i++) {
        REQUIRE(tree.degree(i) == 1);
        REQUIRE(tree.subtreeSize(i) == numberOfLevels + 1 - i);
    }
    REQUIRE(tree.degree(numberOfLevels) == 0);

    for (int level = 0; level < numberOfLevels; level++) {
        tree.deleteNode(1);//delete the child of root.
    }

    REQUIRE(tree.degree(0) == 0);
    REQUIRE(tree.subtreeSize(0) == 1);
    std::vector<bool> expected = {1, 0};
    REQUIRE(tree.getBitString() == expected);

    tree.profiler.print();
}