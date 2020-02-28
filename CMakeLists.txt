cmake_minimum_required(VERSION 3.5)

set(CVPG_VERSION_MAJOR 0)
set(CVPG_VERSION_MINOR 1)
set(CVPG_VERSION_PATCH 0)
set(CVPG_VERSION "${CVPG_VERSION_MAJOR}.${CVPG_VERSION_MINOR}.${CVPG_VERSION_PATCH}")

project(cv-playground VERSION ${CVPG_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_library(${PROJECT_NAME} INTERFACE)

# dependencies
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options system thread)

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# project options
option(BUILD_APPS "Build applications." ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
option(BUILD_TESTS "Build module tests." ON)

# add include files from asynchronous
include_directories(./extern/asynchronous)

# add include files from ChaiScript
include_directories(SYSTEM ./extern/chaiscript/include)

add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(./extern/googletest)

    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    add_subdirectory(test)
endif()
