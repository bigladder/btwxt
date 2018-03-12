# Test Executable
include_directories("../src"
                    "${gtest_SOURCE_DIR}/include")

set(library_sources
  interp_test.cpp
#  scratch.cpp
#  scratch_func2.cpp
)

link_libraries(btwxt gtest)

add_executable(test ${library_sources})
