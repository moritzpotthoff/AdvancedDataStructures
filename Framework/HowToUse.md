# Advanced Data Structures Programming Project

This directory contains all files needed for the implementation project.

## Overview

- The input is read, queries are started and the output is generated in main.cpp.

## Requirements

My implementation is in C++20 and uses CMake. It runs on my Ubuntu 20.04 WSL2 with g++ 10.3.0.

## Building and Execution

To build, run the following commands from the main folder `Framework`:
```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Run the program using 
```
./build/Framework [topk|repeat] path_to_input_file
```

For instance:
```
./build/Framework topk ./TestFiles/topK-trivial.txt
./build/Framework repeat ./TestFiles/repeat-trivial.txt
```
