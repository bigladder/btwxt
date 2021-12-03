# Main library

set(library_sources
  btwxt.h
  btwxt.cpp
  griddeddata.h
  griddeddata.cpp
  gridpoint.h
  gridpoint.cpp
  error.h
  error.cpp
)

option( ${PROJECT_NAME}_STATIC_LIB "Make ${PROJECT_NAME} a static library" ON )

if (${PROJECT_NAME}_STATIC_LIB)
  add_library(${PROJECT_NAME} STATIC ${library_sources})
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-D${PROJECT_NAME}_STATIC_DEFINE")
else()
  set(CMAKE_MACOSX_RPATH 1)
  add_library(${PROJECT_NAME} SHARED ${library_sources})
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})

if (${PROJECT_NAME}_COVERAGE)
  add_coverage(${PROJECT_NAME})
endif()