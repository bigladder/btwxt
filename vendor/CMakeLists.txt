if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    # UPDATE_DISCONNECTED 1: Don't look for remote updates on sub-sequent builds (e.g., git pull)
    # Change to 0 to check for updates to branch
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

include(DownloadProject)
download_project(PROJ                googletest
                 GIT_REPOSITORY      https://github.com/google/googletest.git
                 GIT_TAG             release-1.8.0
                 DOWNLOAD_DIR        "${CMAKE_CURRENT_BINARY_DIR}/googletest-dl"
                 SOURCE_DIR          "${CMAKE_CURRENT_SOURCE_DIR}/googletest"
                 BINARY_DIR          "${CMAKE_CURRENT_BINARY_DIR}/googletest"
                 ${UPDATE_DISCONNECTED_IF_AVAILABLE}
)

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})