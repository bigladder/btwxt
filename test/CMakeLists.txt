# Test Executable
set(library_sources
        test-main.cpp
        public-fixtures.hpp
        btwxt-tests.cpp
        internal-method-tests.cpp
        grid-axis-tests.cpp)

add_executable(${PROJECT_NAME}_tests ${library_sources})

target_compile_features(${PROJECT_NAME}_tests PRIVATE cxx_std_17)

target_link_libraries(${PROJECT_NAME}_tests ${PROJECT_NAME} gtest gmock fmt)

include(GoogleTest)

gtest_discover_tests(${PROJECT_NAME}_tests TEST_PREFIX ${PROJECT_NAME}:)
