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

target_compile_options(btwxt PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
  -Wall -Wextra -Wpedantic>
)
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libs)


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_export.h
              ${CMAKE_CURRENT_SOURCE_DIR}/btwxt.h  
              ${CMAKE_CURRENT_SOURCE_DIR}/error.h  
              ${CMAKE_CURRENT_SOURCE_DIR}/griddeddata.h  
              ${CMAKE_CURRENT_SOURCE_DIR}/gridpoint.h
              DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/btwxt COMPONENT libs)

if (${PROJECT_NAME}_COVERAGE)
  add_coverage(${PROJECT_NAME})
endif()
