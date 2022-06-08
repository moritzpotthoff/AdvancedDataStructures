#include <iostream>
#include <fstream>
#include <cassert>

#include "Helpers/Timer.h"
#include "Helpers/BitVectorProfiler.h"
#include "BitVector/DynamicBitVector.h"
#include "BalancedParentheses/DynamicBP.h"

//Interactive flag. If true, generates a little more output than just the result line.
static const bool Interactive = true;
//Debug flag. Generates extensive debug info.
static const bool Debug = Interactive && false;
static const bool WriteToFile = false;

inline static void handleBitVectorQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested bit vector query." << std::endl;

    //For the evaluation
    Helpers::Timer timer;
    size_t constructionTime = 0, insertTime = 0, deleteTime = 0, flipTime = 0, rankTime = 0, selectTime = 0;
    //Input
    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);
    //Output
    std::string outputFileName(argv[3]);
    std::ofstream outputFile(outputFileName);

    BitVector::DynamicBitVector bv = BitVector::DynamicBitVector<BitVector::BasicProfiler>();

    //Read the bit vector from the file.
    size_t initialLength;
    inputFile >> initialLength;
    for (size_t i = 0; i < initialLength; i++) {
        bool bit;
        inputFile >> bit;
        timer.restart();
        bv.insertBit(i, bit);//TODO use only one operation to insert all initial bits
        constructionTime += timer.getMicroseconds();
    }
    if constexpr (Interactive) std::cout << "Found bit vector of length " << initialLength << "." << std::endl;

    //Read and execute all the queries.
    std::string queryType;
    int index;
    bool bit;
    while (inputFile >> queryType) {
        if (queryType.compare("insert") == 0) {
            inputFile >> index;
            inputFile >> bit;
            timer.restart();
            bv.insertBit(index, bit);
            insertTime += timer.getMicroseconds();
        } else if (queryType.compare("delete") == 0) {
            inputFile >> index;
            timer.restart();
            bv.deleteBit(index);
            deleteTime += timer.getMicroseconds();
        } else if (queryType.compare("flip") == 0) {
            inputFile >> index;
            timer.restart();
            bv.flipBit(index);
            flipTime += timer.getMicroseconds();
        } else if (queryType.compare("rank") == 0) {
            inputFile >> bit;
            inputFile >> index;
            timer.restart();
            const int result = bv.rank(bit, index);
            rankTime += timer.getMicroseconds();
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (Interactive) std::cout << "Rank_" << bit << "[" << index << "]=" << result << std::endl;
        } else if (queryType.compare("select") == 0) {
            inputFile >> bit;
            inputFile >> index;
            timer.restart();
            const int result = bv.select(bit, index);
            selectTime += timer.getMicroseconds();
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (Interactive) std::cout << "Select_" << bit << "[" << index << "]=" << result << std::endl;
        }
    }

    //Print the bit vector to the output file
    std::cout << "BV result:" << std::endl;
    for (int i = 0; i < bv.length; i++) {
        const int result = bv.access(i);
        if constexpr (WriteToFile) outputFile << result << "\n";
        if constexpr (Interactive) std::cout << " BV[" << i << "] = " << result << std::endl;
    }
    std::cout << std::endl << std::endl << std::endl;
    std::cout   << "RESULT algo=bv name=moritz_potthoff"
                << " time=" << (constructionTime + insertTime + deleteTime + flipTime + rankTime + selectTime)
                << " constructionTime=" << constructionTime
                << " insertTime=" << insertTime
                << " deleteTime=" << deleteTime
                << " flipTime=" << flipTime
                << " rankTime=" << rankTime
                << " selectTime=" << selectTime
                << " space=" << "0"//TODO
                << std::endl;

    bv.profiler.print();
}

inline static void handleBPQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested BP query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);

    BalancedParentheses::DynamicBP<BalancedParentheses::NoProfiler> tree;

    //Read and execute all the queries.
    std::string queryType;
    int v, i, k;
    while (inputFile >> queryType) {
        if (queryType.compare("insertchild") == 0) {
            inputFile >> v >> i >> k;
            tree.insertChild(v, i, k);
        } else if (queryType.compare("deletenode") == 0) {
            inputFile >> v;
            tree.deleteNode(v);
        }
    }

    std::cout << "Resulting tree:" << std::endl;
    tree.printTree();
    tree.printBitString();

    //TODO print the degrees of nodes from the resulting tree in preorder dfs order to the output file

    //std::cout << "RESULT algo= name=moritz-potthoff"
              //<< " construction time=" << (preprocessingTime + queryInitTime)//count query initialization as preprocessing: It could be done during the suffix tree generation, if that was only used for repeat queries.
              //<< std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
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

    return 0;
}
