cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
project(FrameWindowCounter VERSION 1.0.0)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)
include($ENV{GEODE_SDK}/cmake/CPM.cmake)

# GDR2 + legacy GDR import.
CPMAddPackage("gh:maxnut/GDReplayFormat#2a0ca3cff9b1d16863037a464da5fe27c28a5823")
CPMAddPackage(
    NAME GDRconverter
    GITHUB_REPOSITORY maxnut/GDR-converter
    GIT_TAG main
    EXCLUDE_FROM_ALL YES
    OPTIONS "BUILD_TESTING OFF"
)

# Silicate SLC v2/v3 reader.
FetchContent_Declare(
    slc_repo
    GIT_REPOSITORY https://github.com/silicate-bot/slc.git
    GIT_TAG main
)
FetchContent_MakeAvailable(slc_repo)

target_link_libraries(${PROJECT_NAME} libGDR GDRconverter)
target_include_directories(${PROJECT_NAME} PRIVATE ${slc_repo_SOURCE_DIR}/include)

setup_geode_mod(${PROJECT_NAME})
