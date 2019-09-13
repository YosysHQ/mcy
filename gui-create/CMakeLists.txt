# TODO: sensible minimum CMake version
cmake_minimum_required(VERSION 3.3)
project(mcy-create)

set(CMAKE_CXX_STANDARD 14)

if (MSVC)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG /W4")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W4")
else()
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -fPIC -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -fPIC -O3 -g")
endif()

if(NOT DEFINED CMAKE_SUPPRESS_DEVELOPER_WARNINGS)
     set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS 1 CACHE INTERNAL "No dev warnings")
endif()

# Find the Qt5 libraries
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_subdirectory(src ${CMAKE_CURRENT_BINARY_DIR}/generated/src)

set(EXECUTABLE_OUTPUT_PATH .)

file(GLOB_RECURSE CLANGFORMAT_FILES *.cc *.h)
string(REGEX REPLACE "[^;]*/generated[^;]*" "" CLANGFORMAT_FILES "${CLANGFORMAT_FILES}")

add_custom_target(
    clangformat
    COMMAND clang-format
    -style=file
    -i
    ${CLANGFORMAT_FILES}
)
