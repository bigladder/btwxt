# Main library

set(library_sources
        regular-grid-interpolator-implementation.h
        regular-grid-interpolator-implementation.cpp
        regular-grid-interpolator.cpp
        grid-axis.cpp
        )

option(${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON)

if (${PROJECT_NAME}_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${library_sources})
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else ()
    set(CMAKE_MACOSX_RPATH 1)
    add_library(${PROJECT_NAME} SHARED ${library_sources})
endif ()

target_link_libraries(${PROJECT_NAME} PUBLIC ${PROJECT_NAME}_interface courier fmt)

target_compile_options(btwxt PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wpedantic>
        )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

if (${PROJECT_NAME}_COVERAGE)
    add_coverage(${PROJECT_NAME})
endif ()

option(BUILD_PYTHON_BINDINGS "Build Python bindings via SWIG" OFF)

set(ENABLE_SWIG OFF)
if (BUILD_PYTHON_BINDINGS)
  set(ENABLE_SWIG ON)
endif()

if(ENABLE_SWIG)
  # lib swig files
  set(${PROJECT_NAME}_swig_src btwxt.i)

  foreach(SWIG_FILE ${${PROJECT_NAME}_swig_src})
    message("SWIG_FILE=${SWIG_FILE}")
    set_source_files_properties(${SWIG_FILE} PROPERTIES
      CPLUSPLUS ON
      INCLUDE_DIRECTORIES ${PROJECT_SOURCE_DIR}/include
      USE_TARGET_INCLUDE_DIRECTORIES ON
    )
  endforeach()

  find_package(SWIG REQUIRED COMPONENTS python)
  message("${SWIG_USE_FILE}")
  include(${SWIG_USE_FILE})
endif()

if (BUILD_PYTHON_BINDINGS)

  set(swig_target_name ${PROJECT_NAME}_python)

  # **Only** find static libs
  set(Python_USE_STATIC_LIBS TRUE)

  find_package(
    Python
    COMPONENTS Interpreter Development
    REQUIRED)

  swig_add_library(
    ${swig_target_name}
    TYPE SHARED
    LANGUAGE python OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/python"
                    OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/python_wrapper"
    SOURCES ${${PROJECT_NAME}_swig_src})

  set_target_properties(${swig_target_name} PROPERTIES OUTPUT_NAME
                                                       ${PROJECT_NAME})

  set_target_properties(
    ${swig_target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
                                   "${CMAKE_CURRENT_BINARY_DIR}/python/")
  set_target_properties(
    ${swig_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY
                                   "${CMAKE_CURRENT_BINARY_DIR}/python/")
  set_target_properties(
    ${swig_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                   "${CMAKE_CURRENT_BINARY_DIR}/python/")

   target_include_directories(${swig_target_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
   target_include_directories(${swig_target_name} PRIVATE SYSTEM ${Python_INCLUDE_DIRS})

  swig_link_libraries(${swig_target_name}
    PUBLIC ${PROJECT_NAME}
  )

  if(MSVC)
    swig_link_libraries(${swig_target_name} PRIVATE Python::Module)
  endif()

  if (${PROJECT_NAME}_BUILD_TESTING)
    set(test_case_name btwxt_python)
    add_test(
          NAME "${test_case_name}"
          COMMAND ${Python_EXECUTABLE} -m pytest --verbose test_btwxt.py
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
    )
    # Pytest Exit code 5 means all tests were deselected
    set_tests_properties("${test_case_name}" PROPERTIES SKIP_RETURN_CODE 5)
    set_tests_properties("${test_case_name}" PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python/")
  endif()
endif()
