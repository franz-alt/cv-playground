cmake_minimum_required(VERSION 3.12)

set(CVPG_VERSION_MAJOR 0)
set(CVPG_VERSION_MINOR 1)
set(CVPG_VERSION_PATCH 0)
set(CVPG_VERSION "${CVPG_VERSION_MAJOR}.${CVPG_VERSION_MINOR}.${CVPG_VERSION_PATCH}")

include(${CMAKE_SOURCE_DIR}/cmake/project_settings.cmake)

project(cv-playground VERSION ${CVPG_VERSION} LANGUAGES CXX)

# force C++ standard without fall-back and without compiler extensions
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

add_library(${PROJECT_NAME} INTERFACE)

# dependencies
find_package(Threads REQUIRED)
find_package(PNG REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options system thread)
find_package(Git QUIET)

# set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

# project options
option(BUILD_APPS "Build applications." ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
option(BUILD_TESTS "Build module tests." ON)

option(BUILD_WITH_TCMALLOC "Build the software with TCMalloc." OFF)

if(BUILD_WITH_TCMALLOC)
    find_package(TCMalloc REQUIRED)

    add_definitions(-DUSE_TCMALLOC)
endif()

option(BUILD_WITH_FFMPEG "Build with enabled FFmpeg support for video processing." ON)

if(BUILD_WITH_FFMPEG)
    find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE)
endif()

option(BUILD_WITH_TENSORFLOW_SEGMENTATION "Build with enabled Google TensorFlow C++ support for image segmentation and enable 'tfpredict' algorithm." OFF)

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    find_package(TensorFlowCC REQUIRED)

    if(TensorFlowCC_FOUND)
        add_definitions(-DUSE_TENSORFLOW_CC)
    elseif()
        message(FATAL_ERROR "TensorFlow C++ libraries required, but not found!")
    endif()
endif()

# sanitizer handling
include(cmake/sanitizers.cmake)

if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)

    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")

        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)

        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

# check if ChaiScript was downloaded properly
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/chaiscript/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# check if googletest was downloaded properly
if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/googletest/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

# add include files from asynchronous
include_directories(${PROJECT_SOURCE_DIR}/extern/asynchronous)

# add include files from ChaiScript
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/extern/chaiscript/include)

add_subdirectory(src)

if(BUILD_TESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/extern/googletest)

    enable_testing()
    include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

    add_subdirectory(test)
endif()
