# AdvancedDataStructures

## Building and Execution

To build, run the following commands from the main folder `Framework`:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the program using 
```
./build/Framework [bv|bp] path_to_input_file path_to_output_file
```

## Overview over the project

- The input is read, queries are started, and the output is generated in main.cpp.
- My implementations follow the data structures described by Gonzalo Navarro in his book Compact Data Structures - A Practical Approach. Cambridge University Press, 2016. 
- For efficiency, I have separate implementations of the bit vector for the bv- and bp-part of the exercise.
- I used Profilers during the optimization of the queries. The current variant uses the dummy variants to avoid overheads.
- I used Catch (https://github.com/catchorg/Catch2) for automatic testing. Tests can be found in `/Tests/`. Currently, they are not compiled as the test framework significantly increases compile times.
- In `main.cpp`, there is also some code to generate evaluation output that I used to generate plots for the presentation.

### Dynamic Bit Vectors
- Within leaves, I store the bits as a sequence of uint64_t integers with 64 bits each. Within integers, bits are stored from left to right (the most valuable bit is bit 0). This is implemented in `*/InnerBitVectorByInt.h`
- There is a previous version of the inner bit vector using a simple `std::vector<bool>`. It can be found in `*/InnerBitVector.h`, but is not used in the current configuration.  
- As in the source, leaves in the AVL tree store between w^2/2 and 2*w^2 bits. w is a parameter (`Definitions.h`) that can be selected for performance tuning. Larger values significantly increase running times while notably reducing memory overheads. Currently, it is tuned to be very space-efficient with high running times accepted because of the competition metric.
- Initially, the AVL-tree bit vector is generated from the bit vector that is contained in the input file by generating a complete binary tree, rather than performing individual inserts. This yields better performance and leaves that contain w^2 bits each which speeds up subsequent inserts and deletes (less merges/splits).

### Dynamic BP-Trees
- All bp-queries operate on indices rather than preorder numbers. Hence, they require additional rank- or select-calls (in main.cpp) for the desired output format.
- Opposed to Navarro's book, I do not use the precomputed C-array to (re)compute excess values more efficiently; instead, I do this naively.

## Requirements

My implementation is in C++20 and uses CMake. It runs on my Ubuntu 20.04 WSL2 with g++ 10.3.0.