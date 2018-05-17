# this is where there will be build instructions
# https://github.com/cse/btwxt
cmake_minimum_required(VERSION 3.10)

project(btwxt)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

add_subdirectory(src)
add_subdirectory(vendor)

include(ctest)

add_subdirectory(test)
