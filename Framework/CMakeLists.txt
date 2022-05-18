project(Framework)

set(CMAKE_CXX_STANDARD 20)

if(NOT MARCH)
    set(MARCH native)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-D_GLIBCXX_DEBUG -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-ffast-math -ftree-vectorize -Wfatal-errors -DNDEBUG -O3")

add_executable(Framework main.cpp)