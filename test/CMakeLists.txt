# Test Executable

include_directories("${PROJECT_SOURCE_DIR}/src"
                    "${gtest_SOURCE_DIR}/include")

set(library_sources
  main_test.cpp
  fixtures.hpp
  btwxt_test.cpp
  griddeddata_test.cpp
  gridpoint_test.cpp
  math_test.cpp
  large_test.cpp
  hypercube_test.cpp
)


add_executable(${PROJECT_NAME}_tests ${library_sources})

target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME} gtest gmock btwxt_interface_library)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME}_tests TEST_PREFIX ${PROJECT_NAME}:)
