cmake_minimum_required(VERSION 2.8)
project("FastCrawl" CXX)
enable_testing()

# Version
set(FASTCRAWL_VERSION_MAJOR 0)
set(FASTCRAWL_VERSION_MINOR 1)
set(FASTCRAWL_VERSION_PATCH 0)

# Config. header file
configure_file(
    "${PROJECT_SOURCE_DIR}/libfastcrawl/config.hxx.in"
    "${PROJECT_BINARY_DIR}/libfastcrawl/config.hxx")

# C++ flags
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_SCC_STANDARDREQUIRED ON)
set(CMAKE_CXX_FLAGS "-g -Wall -Werror -O2 ${CMAKE_CXX_FLAGS} ${CXXFLAGS}")

# Dependencies
find_library(PTHREAD NAMES pthread REQUIRED)
find_library(RT NAMES rt REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

# Strip source directory prefix from file names
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
    -D__SRCFILE__='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

# Top-level include path
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

# Allow inclusion of the config. header file
include_directories("${PROJECT_BINARY_DIR}/libfastcrawl")

# fastcrawl library
add_subdirectory(libfastcrawl)

# fastcrawl CLI
add_executable(fcrawl fcrawl.cxx)
target_link_libraries(fcrawl
    LINK_PUBLIC fastcrawl
    LINK_PUBLIC curl)
    #LINK_PUBLIC zlib)
