# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.25

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/phil-ahrenkiel/Documents/GitHub/btwxt

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug

# Utility rule file for courierr.

# Include any custom commands dependencies for this target.
include vendor/courierr/include/courierr/CMakeFiles/courierr.dir/compiler_depend.make

# Include the progress variables for this target.
include vendor/courierr/include/courierr/CMakeFiles/courierr.dir/progress.make

courierr: vendor/courierr/include/courierr/CMakeFiles/courierr.dir/build.make
.PHONY : courierr

# Rule to build all files generated by this target.
vendor/courierr/include/courierr/CMakeFiles/courierr.dir/build: courierr
.PHONY : vendor/courierr/include/courierr/CMakeFiles/courierr.dir/build

vendor/courierr/include/courierr/CMakeFiles/courierr.dir/clean:
	cd /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug/vendor/courierr/include/courierr && $(CMAKE_COMMAND) -P CMakeFiles/courierr.dir/cmake_clean.cmake
.PHONY : vendor/courierr/include/courierr/CMakeFiles/courierr.dir/clean

vendor/courierr/include/courierr/CMakeFiles/courierr.dir/depend:
	cd /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/phil-ahrenkiel/Documents/GitHub/btwxt /Users/phil-ahrenkiel/Documents/GitHub/btwxt/vendor/courierr/include/courierr /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug/vendor/courierr/include/courierr /Users/phil-ahrenkiel/Documents/GitHub/btwxt/cmake-build-debug/vendor/courierr/include/courierr/CMakeFiles/courierr.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : vendor/courierr/include/courierr/CMakeFiles/courierr.dir/depend
