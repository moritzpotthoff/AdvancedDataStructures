# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build

# Include any dependencies generated for this target.
include CMakeFiles/Framework.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/Framework.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/Framework.dir/flags.make

CMakeFiles/Framework.dir/main.cpp.o: CMakeFiles/Framework.dir/flags.make
CMakeFiles/Framework.dir/main.cpp.o: ../main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/Framework.dir/main.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/Framework.dir/main.cpp.o -c /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/main.cpp

CMakeFiles/Framework.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/Framework.dir/main.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/main.cpp > CMakeFiles/Framework.dir/main.cpp.i

CMakeFiles/Framework.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/Framework.dir/main.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/main.cpp -o CMakeFiles/Framework.dir/main.cpp.s

# Object files for target Framework
Framework_OBJECTS = \
"CMakeFiles/Framework.dir/main.cpp.o"

# External object files for target Framework
Framework_EXTERNAL_OBJECTS =

Framework: CMakeFiles/Framework.dir/main.cpp.o
Framework: CMakeFiles/Framework.dir/build.make
Framework: CMakeFiles/Framework.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable Framework"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/Framework.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/Framework.dir/build: Framework

.PHONY : CMakeFiles/Framework.dir/build

CMakeFiles/Framework.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/Framework.dir/cmake_clean.cmake
.PHONY : CMakeFiles/Framework.dir/clean

CMakeFiles/Framework.dir/depend:
	cd /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build /mnt/c/Users/Moritz/Documents/Studium/Textindexierung/TextIndexing/Framework/build/CMakeFiles/Framework.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/Framework.dir/depend
