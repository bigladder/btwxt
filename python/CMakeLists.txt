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

  set(Python_REQUIRED_VERSION "" CACHE STRING "The specific Python version you want to link to (Optional)")
  set(Python_ROOT_DIR "" CACHE PATH "The path to the root installation of your python version, if you want to explicitly set it (Optional)")
  mark_as_advanced(Python_REQUIRED_VERSION)
  mark_as_advanced(Python_ROOT_DIR)
  if(Python_REQUIRED_VERSION)
    find_package(Python ${Python_REQUIRED_VERSION} EXACT COMPONENTS Interpreter Development REQUIRED)
  else()
    find_package(Python 3.8 COMPONENTS Interpreter Development REQUIRED)
  endif()

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
    LANGUAGE python
    OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/btwxt"
    OUTFILE_DIR "${CMAKE_CURRENT_BINARY_DIR}/btwxt_wrapper"
    SOURCES ${${PROJECT_NAME}_swig_src}
  )

  set_target_properties(${swig_target_name} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

  set_target_properties(${swig_target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/btwxt/")
  set_target_properties(${swig_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/btwxt/")
  set_target_properties(${swig_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/btwxt/")

  target_include_directories(${swig_target_name} PRIVATE ${PROJECT_SOURCE_DIR}/include)
  target_include_directories(${swig_target_name} PRIVATE SYSTEM ${Python_INCLUDE_DIRS})

  swig_link_libraries(${swig_target_name}
    PUBLIC ${PROJECT_NAME}
  )

  if(MSVC OR APPLE)
    swig_link_libraries(${swig_target_name} PRIVATE Python::Module)
  endif()
  if(MSVC)
    set_target_properties(${swig_target_name} PROPERTIES SUFFIX ".pyd")
    # Avoid having a python/btwxt/Release/*.pyd path.
    set_target_properties(${swig_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
    set_target_properties(${swig_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
    set_target_properties(${swig_target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
    set_target_properties(${swig_target_name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
    set_target_properties(${swig_target_name} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
    set_target_properties(${swig_target_name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_BINARY_DIR}/btwxt")
  elseif(APPLE)
    set_target_properties(${swig_target_name} PROPERTIES SUFFIX ".so")
  endif()

  set(PYPI_VERSION 1.0.0)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py ${CMAKE_CURRENT_BINARY_DIR}/setup.py @ONLY)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/btwxt.py ${CMAKE_CURRENT_BINARY_DIR}/btwxt/__init__.py COPYONLY)
  set(PYTHON_PIP_REPOSITORY testpypi)

  add_custom_target(python_package
    COMMAND ${CMAKE_COMMAND} -E rm -Rf "build/" "dist/" "btwxt.egg-info/"
    COMMAND ${CMAKE_COMMAND} -E echo "To create the package, cd 'python/'"
    COMMAND ${CMAKE_COMMAND} -E echo "${Python_EXECUTABLE} setup.py bdist_wheel"
    COMMAND ${Python_EXECUTABLE} setup.py bdist_wheel
    COMMAND ${CMAKE_COMMAND} -E echo "To upload the package, cd 'python/'"
    COMMAND ${CMAKE_COMMAND} -E echo "${Python_EXECUTABLE} -m twine upload --verbose --skip-existing --repository ${PYTHON_PIP_REPOSITORY} dist/*"
    DEPENDS
      ${swig_target_name}
      ${ALL_PYTHON_PACKAGE_TARGETS}
      ${ALL_PYTHON_PACKAGE_GENERATED_SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

  if (${PROJECT_NAME}_BUILD_TESTING)
    set(test_case_name btwxt_python)
    add_test(
          NAME "${test_case_name}"
          COMMAND ${Python_EXECUTABLE} -m pytest --verbose test_btwxt.py
          WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
    )
    # Pytest Exit code 5 means all tests were deselected
    set_tests_properties("${test_case_name}" PROPERTIES SKIP_RETURN_CODE 5)
    set_tests_properties("${test_case_name}" PROPERTIES ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/btwxt")
  endif()
endif()
