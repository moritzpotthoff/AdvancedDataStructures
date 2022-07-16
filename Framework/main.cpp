#include <iostream>
#include <fstream>
#include <cassert>
#include <climits>
#include <stack>

//for testing
/*
#define CATCH_CONFIG_RUNNER
#include "Tests/catch.hpp"
#include "Tests/TestBP.h"
#include "Tests/TestBV.h"
#include "Tests/TestIntInner.h"
*/

#include "Definitions.h"
#include "Helpers/Timer.h"
#include "Helpers/Asserts.h"
#include "Helpers/BitVectorProfiler.h"
#include "BitVector/DynamicBitVector.h"
#include "BitVector/InnerBitVector.h"
#include "BitVector/InnerBitVectorByInt.h"
#include "BitVector/Node.h"
#include "BalancedParentheses/DynamicBP.h"
#include "BalancedParentheses/Node.h"
#include "BalancedParentheses/InnerBitVector.h"
#include "BalancedParentheses/InnerBitVectorByInt.h"

//Interactive flag. If true, generates a little more output than just the result line.
static const bool Interactive = true;
static const bool VeryInteractive = false;
static const bool WriteToFile = true;
static const bool EvaluationMode = false;

inline static void handleBitVectorQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested bit vector query." << std::endl;

    //For the evaluation
    Helpers::Timer timer;
    double constructionTime = 0, insertTime = 0, deleteTime = 0, flipTime = 0, rankTime = 0, selectTime = 0;
    size_t inserts = 0, deletes = 0, flips = 0, ranks = 0, selects = 0;
    //Input
    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);
    //Output
    std::string outputFileName(argv[3]);
    std::ofstream outputFile(outputFileName);


    //Read the bit vector from the file.
    size_t initialLength;
    inputFile >> initialLength;
    std::vector<bool> bits;
    bits.reserve(initialLength);
    for (size_t i = 0; i < initialLength; i++) {
        bool bit;
        inputFile >> bit;
        bits.emplace_back(bit);
    }
    timer.restart();
    BitVector::DynamicBitVector<BitVector::NoProfiler, BitVector::InnerBitVectorByInt> bv(bits);
    constructionTime += timer.getMicroseconds();
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
            inserts++;
        } else if (queryType.compare("delete") == 0) {
            inputFile >> index;
            timer.restart();
            bv.deleteBit(index);
            deleteTime += timer.getMicroseconds();
            deletes++;
        } else if (queryType.compare("flip") == 0) {
            inputFile >> index;
            timer.restart();
            bv.flipBit(index);
            flipTime += timer.getMicroseconds();
            flips++;
        } else if (queryType.compare("rank") == 0) {
            inputFile >> bit;
            inputFile >> index;
            timer.restart();
            const int result = bv.rank(bit, index);
            rankTime += timer.getMicroseconds();
            ranks++;
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (VeryInteractive) std::cout << "Rank_" << bit << "[" << index << "]=" << result << std::endl;
        } else if (queryType.compare("select") == 0) {
            inputFile >> bit;
            inputFile >> index;
            timer.restart();
            const int result = bv.select(bit, index);
            selectTime += timer.getMicroseconds();
            selects++;
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (VeryInteractive) std::cout << "Select_" << bit << "[" << index << "]=" << result << std::endl;
        }
    }

    if constexpr (VeryInteractive) {
        std::cout << "BV result:" << std::endl;
        for (int i = 0; i < bv.length; i++) {
            const int result = bv.access(i);
            std::cout << " BV[" << i << "] = " << result << std::endl;
        }
    }

    const int totalTime = constructionTime + insertTime + deleteTime + flipTime + rankTime + selectTime;
    std::cout   << "RESULT algo=bv name=moritz_potthoff"
                << " time=" << totalTime / 1000
                << " space=" << bv.getSize() << std::endl;

    if constexpr (EvaluationMode) {
        std::cout   << "RESULT algo=bv name=moritz_potthoff"
                    << " time=" << totalTime / 1000
                    << " constructionTimePer10kBits=" << constructionTime / (initialLength / 10000)
                    << " insertTime=" << ((inserts > 0) ? insertTime / inserts : -1)
                    << " deleteTime=" << ((deletes > 0) ? deleteTime / deletes : -1)
                    << " flipTime=" << ((flips > 0) ? flipTime / flips : -1)
                    << " rankTime=" << ((ranks > 0) ? rankTime / ranks : -1)
                    << " selectTime=" << ((selects > 0) ? selectTime / selects : -1)
                    << " input=" << inputFileName
                    << " w=" << w
                    << " space=" << bv.getSize()
                    << " length=" << bv.length
                    << " overhead=" << (double)bv.getSize() / (double)bv.length
                    << " score=" << 0.45 * (totalTime / 1000) + 0.55 * bv.getSize() << std::endl;
        std::cout   << "RESULT evalType=totalTime resultType=total" << " w=" << w << " input=" << inputFileName
                    << " time=" << totalTime << std::endl;
        std::cout   << "RESULT evalType=totalTime resultType=construction" << " w=" << w << " input=" << inputFileName
                    << " time=" << constructionTime / (initialLength / 10000) << std::endl;
        std::cout   << "RESULT evalType=queryTime resultType=insert" << " w=" << w << " input=" << inputFileName
                    << " time=" << ((inserts > 0) ? insertTime / inserts : -1) << std::endl;
        std::cout   << "RESULT evalType=queryTime resultType=delete" << " w=" << w << " input=" << inputFileName
                    << " time=" << ((deletes > 0) ? deleteTime / deletes : -1) << std::endl;
        std::cout   << "RESULT evalType=queryTime resultType=rank" << " w=" << w << " input=" << inputFileName
                    << " time=" << ((ranks > 0) ? rankTime / ranks : -1) << std::endl;
        std::cout   << "RESULT evalType=queryTime resultType=select" << " w=" << w << " input=" << inputFileName
                    << " time=" << ((selects > 0) ? selectTime / selects : -1) << std::endl;
        std::cout   << "RESULT evalType=overhead resultType=overhead" << " w=" << w << " input=" << inputFileName
                    << " overhead=" << (double)bv.getSize() / (double)bv.length << std::endl;
        std::cout   << "RESULT evalType=score resultType=score" << " w=" << w << " input=" << inputFileName
                    << " score=" << 0.45 * (totalTime / 1000) + 0.55 * bv.getSize() << std::endl;
    }

    if constexpr (Interactive) bv.profiler.print();
}

inline static void handleBPQuery(char *argv[]) {
    if constexpr (Interactive) std::cout << "Requested BP query." << std::endl;

    std::string inputFileName(argv[2]);
    std::ifstream inputFile(inputFileName);
    //Output
    std::string outputFileName(argv[3]);
    std::ofstream outputFile(outputFileName);

    BalancedParentheses::DynamicBP<BalancedParentheses::BasicProfiler, BalancedParentheses::InnerBitVectorByInt> tree;
    Helpers::Timer timer;
    size_t time = 0;

    //Read and execute all the queries.
    std::string queryType;
    int v, i, k;
    while (inputFile >> queryType) {
        if (queryType.compare("insertchild") == 0) {
            inputFile >> v >> i >> k;
            timer.restart();
            tree.insertChild(tree.getIndex(v), i, k);
            time += timer.getMicroseconds();
        } else if (queryType.compare("deletenode") == 0) {
            inputFile >> v;
            timer.restart();
            tree.deleteNode(tree.getIndex(v));
            time += timer.getMicroseconds();
        } else if (queryType.compare("child") == 0) {
            inputFile >> v >> i;
            timer.restart();
            const int result = tree.getNumber(tree.child(tree.getIndex(v), i));
            time += timer.getMicroseconds();
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (VeryInteractive) std::cout << "child(" << v << ", " << i << ") = " << result << std::endl;
        } else if (queryType.compare("subtree_size") == 0) {
            inputFile >> v;
            timer.restart();
            const int result = tree.subtreeSize(tree.getIndex(v));
            time += timer.getMicroseconds();
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (VeryInteractive) std::cout << "subtreeSize(" << v << ") = " << result << std::endl;
        } else if (queryType.compare("parent") == 0) {
            inputFile >> v;
            timer.restart();
            const int result = tree.getNumber(tree.parent(tree.getIndex(v)));
            time += timer.getMicroseconds();
            if constexpr (WriteToFile) outputFile << result << "\n";
            if constexpr (VeryInteractive) std::cout << "parent(" << v << ") = " << result << std::endl;
        }
    }

    if constexpr (WriteToFile) tree.printDegreesToFile(outputFile);

    std::cout << "RESULT algo=bp name=moritz-potthoff"
              << " time=" << time / 1000
              << " space=" << tree.getSize()
              << " score=" << 0.45 * (time / 1000) + 0.55 * tree.getSize() << std::endl;

    if constexpr (Interactive) tree.profiler.print();
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        //testing
        //int result = Catch::Session().run(argc, argv);
        //return result;
        return 0;
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
