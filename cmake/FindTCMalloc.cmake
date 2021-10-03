# Find the TCMalloc includes and library
#
# Once done this will be defined:
#
#   TCMalloc_FOUND       - true if TCMalloc found
#   TCMalloc_INCLUDE_DIR - include directory of TCMalloc
#   TCMalloc_LIBRARIES   - TCMalloc libraries
#

find_path(TCMalloc_INCLUDE_DIR google/tcmalloc.h NO_DEFAULT_PATH PATHS
    /usr/include
    /opt/local/include
    /usr/local/include
)

find_library(TCMalloc_LIBRARY #NO_DEFAULT_PATH
    NAMES tcmalloc
    #PATHS /lib /usr/lib /usr/local/lib /opt/local/lib
)

if(TCMalloc_INCLUDE_DIR AND TCMalloc_LIBRARY)
    set(TCMalloc_FOUND TRUE)
    set(TCMalloc_LIBRARIES ${TCMalloc_LIBRARY})
else()
    set(TCMalloc_FOUND FALSE)
    set(TCMalloc_LIBRARIES)
endif()

if(TCMalloc_FOUND)
    message(STATUS "Found TCMalloc: ${TCMalloc_LIBRARY}")
else()
    message(STATUS "Not Found TCMalloc: ${TCMalloc_LIBRARY}")

    if(TCMalloc_FIND_REQUIRED)
        message(STATUS "Looked for TCMalloc libraries named ${TCMalloc_NAMES}.")
        message(FATAL_ERROR "Could NOT find TCMalloc library")
    endif()
endif()

mark_as_advanced(
    TCMalloc_LIBRARY
    TCMalloc_INCLUDE_DIR
)
