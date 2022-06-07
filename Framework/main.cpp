#include <iostream>
#include <fstream>

#include "Helpers/Timer.h"
#include "BitVector/DynamicBitVector.h"

//Interactive flag. If true, generates a little more output than just the result line.
static const bool Interactive = true;
//Debug flag. Generates extensive debug info.
static const bool Debug = Interactive && false;
static const bool WriteToFile = false;

inline static void handleBitVectorQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested bit vector query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    std::string outputFileName(argv[3]);
    std::ofstream outputFile(outputFileName);

    BitVector::DynamicBitVector bv = BitVector::DynamicBitVector();

    //Read the bit vector from the file.
    size_t initialLength;
    inputFile >> initialLength;
    for (size_t i = 0; i < initialLength; i++) {
        bool bit;
        inputFile >> bit;
        bv.insertBit(i, bit);//TODO use only one operation to insert all initial bits
    }
    if constexpr (Interactive) std::cout << "Found bit vector of length " << initialLength << "." << std::endl;

    //Todo read and execute all the queries
    std::string queryType;
    int index;
    bool bit;
    while (inputFile >> queryType) {
        if (queryType.compare("insert") == 0) {
            inputFile >> index;
            inputFile >> bit;
            bv.insertBit(index, bit);
        } else if (queryType.compare("delete") == 0) {
            inputFile >> index;
            bv.deleteBit(index);
        } else if (queryType.compare("flip") == 0) {
            inputFile >> index;
            bv.flipBit(index);
        } else if (queryType.compare("rank") == 0) {
            inputFile >> bit;
            inputFile >> index;
            const int result = bv.rank(bit, index);
            if constexpr (WriteToFile) outputFile << result << "\n";
            std::cout << "Rank_" << bit << "[" << index << "]=" << result << std::endl;
        } else if (queryType.compare("select") == 0) {
            inputFile >> bit;
            inputFile >> index;
            const int result = bv.select(bit, index);
            if constexpr (WriteToFile) outputFile << result << "\n";
            std::cout << "Select_" << bit << "[" << index << "]=" << result << std::endl;
        }
    }

    //Todo print the bit vector to the output file
    std::cout << "BV result:" << std::endl;
    for (int i = 0; i < bv.length; i++) {
        const int result = bv.access(i);
        if constexpr (WriteToFile) outputFile << result << "\n";
        std::cout << " BV[" << i << "] = " << result << std::endl;
    }

    /*
    std::cout   << "RESULT algo=bv name=moritz_potthoff"
                << " time=" << "0"
                << " space=" << "0"
                << std::endl;
    */
}

inline static void handleBPQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested BP query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //TODO read and execute all the queries

    //TODO print the degrees of nodes from the resulting tree in preorder dfs order to the output file

    std::cout << "RESULT algo= name=moritz-potthoff"
              //<< " construction time=" << (preprocessingTime + queryInitTime)//count query initialization as preprocessing: It could be done during the suffix tree generation, if that was only used for repeat queries.
              << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Wrong number of arguments, expecting 3 arguments." << std::endl;
        return 1;
    }

    std::string queryChoice(argv[1]);
    if (queryChoice.compare("bv") == 0) {
        handleBitVectorQuery(argv);
    } else if (queryChoice.compare("bp") == 0) {
        handleBPQuery(argv);
    } else {
        std::cout << "Unknown query choice." << std::endl;
        return 1;
    }

     /*

    BitVector::DynamicBitVector bv;
    for (int i = 0; i < 200; i++) {
        bv.insertBit(i, false);
        //std::cout << std::endl << std::endl << "After insert " << i << " the tree is:" << std::endl;
        //bv.printTree();
        //std::cout << std::endl << std::endl;
    }

    bv.flipBit(10);
    bv.flipBit(20);
    bv.flipBit(30);
    bv.flipBit(40);
    bv.flipBit(120);
    bv.flipBit(130);

    bv.printBitString();
    bv.printTree();

    bv.deleteBit(10);
    bv.deleteBit(50 - 1);
    bv.deleteBit(80 - 2);
    bv.deleteBit(90 - 3);
    bv.deleteBit(100 - 4);
    bv.deleteBit(120 - 5);

    bv.printBitString();
    bv.printTree();

    std::cout << "TEST RANK QUERIES" << std::endl;
    for (int i = 0; i < 200; i++) {
        std::cout << "  Index " << i << " has rankOne of " << bv.rankOne(i) << std::endl;
    }

    std::cout << "TEST ACCESS QUERIES" << std::endl;
    for (int i = 0; i < 202; i++) {
        std::cout << "  Index " << i << " has value of " << bv.access(i) << std::endl;
    }

    std::cout << "TEST SELECT_1 QUERIES" << std::endl;
    for (int i = 1; i <= 4; i++) {
        std::cout << "  SELECT_1 " << i << " has value of " << bv.selectOne(i) << std::endl;
    }

    std::cout << "TEST SELECT_0 QUERIES" << std::endl;
    for (int i = 1; i < 190; i++) {
        std::cout << "  SELECT_0 " << i << " has value of " << bv.selectZero(i) << std::endl;
    }
    */

    return 0;
}
