# common project header to be used in root project files before or right after the 'project()' command

# set the default build type to release
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Build type not set. Set to 'Release' as default")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Select the build type." FORCE)
endif()

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILDTYPE)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(TARGET_PREFIX "d")
endif()

# set global library folder
if(NOT LIBRARY_FOLDER)
    set(LIBRARY_FOLDER ${PROJECT_NAME_LOWERCASE})
endif()

# installation directory of libraries
set(INSTALL_LIB_DIR lib CACHE PATH "Relative installation path for libraries")

# installation directory of binaries
set(INSTALL_BIN_DIR bin CACHE PATH "Relative installation path for executables")

# installation directory of headers
set(INSTALL_INCLUDE_DIR include CACHE PATH "Relative installation path for header files")
