# Test Executable
include_directories("../src"
                    "${gtest_SOURCE_DIR}/include"
                    "${PROJECT_SOURCE_DIR}/vendor/eigen3")

set(library_sources
  main_test.cpp
  btwxt_test.cpp
)

link_libraries(btwxt gtest gmock)

add_executable(test ${library_sources})
