# TODO: sensible minimum CMake version
cmake_minimum_required(VERSION 3.13)
project(mcy-gui)

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

# List of Boost libraries to include
set(boost_libs filesystem)
find_package(Boost REQUIRED COMPONENTS ${boost_libs})
# Detect Qt version (try Qt6 first)
find_package(Qt6 COMPONENTS Core Widgets Sql QUIET)
if (Qt6_FOUND)
    message(STATUS "Using Qt6")
    set(QT_VERSION_MAJOR 6)
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Sql)
else()
    message(STATUS "Using Qt5")
    find_package(Qt5 COMPONENTS Core Widgets Sql REQUIRED)
    set(QT_VERSION_MAJOR 5)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Sql)
    if(APPLE)
        find_package(Qt5 COMPONENTS MacExtras REQUIRED)
        list(APPEND QT_LIBRARIES Qt5::MacExtras)
    endif()
endif()

add_subdirectory(3rdparty/QScintilla ${CMAKE_CURRENT_BINARY_DIR}/generated/3rdparty/QScintilla)
add_subdirectory(3rdparty/QtPropertyBrowser ${CMAKE_CURRENT_BINARY_DIR}/generated/3rdparty/QtPropertyBrowser EXCLUDE_FROM_ALL)
add_subdirectory(src ${CMAKE_CURRENT_BINARY_DIR}/generated/src)

set(EXECUTABLE_OUTPUT_PATH .)

file(GLOB_RECURSE CLANGFORMAT_FILES *.cc *.h)
string(REGEX REPLACE "[^;]*/3rdparty[^;]*" "" CLANGFORMAT_FILES "${CLANGFORMAT_FILES}")
string(REGEX REPLACE "[^;]*/generated[^;]*" "" CLANGFORMAT_FILES "${CLANGFORMAT_FILES}")

add_custom_target(
    clangformat
    COMMAND clang-format
    -style=file
    -i
    ${CLANGFORMAT_FILES}
)
