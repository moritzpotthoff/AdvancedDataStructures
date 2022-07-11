cmake_minimum_required(VERSION 3.13)

project(Framework)

set(CMAKE_CXX_STANDARD 20)

if(NOT MARCH)
    set(MARCH native)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-D_GLIBCXX_DEBUG -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-ffast-math -ftree-vectorize -Wfatal-errors -DNDEBUG -O3")

include_directories(.)
include_directories(BalancedParentheses)
include_directories(BitVector)
include_directories(Helpers)
include_directories(Tests)

add_executable(Framework
        BalancedParentheses/DynamicBP.h
        BalancedParentheses/InnerBitVector.h
        BalancedParentheses/Node.h
        Definitions.h
        BitVector/DynamicBitVector.h
        BitVector/InnerBitVector.h
        BitVector/Node.h
        Helpers/Asserts.h
        Helpers/BitVectorProfiler.h
        Helpers/BpProfiler.h
        Helpers/Timer.h
        Tests/catch.hpp
        Tests/TestBP.h
        main.cpp)
