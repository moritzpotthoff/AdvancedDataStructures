#include <iostream>
#include <fstream>

#include "Helpers/Timer.h"
#include "BitVector/DynamicBitVector.h"

//Interactive flag. If true, generates a little more output than just the result line.
static const bool Interactive = false;
//Debug flag. Generates extensive debug info.
static const bool Debug = Interactive && false;

inline static void handleBitVectorQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested bit vector query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    //Read the bit vector from the file.
    size_t initialLength;
    inputFile >> initialLength;
    for (size_t i = 0; i < initialLength; i++) {
        bool bit;
        inputFile >> bit;
        //Todo store the bits
    }
    if constexpr (Interactive) std::cout << "Found bit vector of length " << initialLength << "." << std::endl;

    //Todo read and execute all the queries

    //Todo print the bit vector to the output file

    std::cout   << "RESULT algo=bv name=moritz_potthoff"
                << " time=" << "0"
                << " space=" << "0"
                << std::endl;
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
    /*if (argc < 4) {
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
    }*/

    BitVector::DynamicBitVector bv;
    for (int i = 0; i < 234; i++) {
        bv.insertBit(i, (i % 2 == 0));
        std::cout << std::endl << std::endl << "After insert " << i << " the tree is:" << std::endl;
        bv.printTree();
        std::cout << std::endl << std::endl;
    }

    bv.printBitString();

    bv.printTree();

    return 0;
}
