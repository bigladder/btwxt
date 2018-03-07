# Test Executable
include_directories("../src")

set(library_sources
  interp_test.cpp
)

link_libraries(bl_interp)

add_executable(test ${library_sources})
