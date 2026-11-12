include(FetchContent)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.10.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(benchmark)

add_executable(${PROJECT_NAME}_benchmarks btwxt-benchmarks.cpp)
target_compile_features(${PROJECT_NAME}_benchmarks PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME}_benchmarks PRIVATE ${PROJECT_NAME} benchmark::benchmark)
