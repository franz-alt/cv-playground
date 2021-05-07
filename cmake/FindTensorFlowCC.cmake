#
# Try to find the TensorFlow C++ libraries. Set the variable 'TensorFlowCC_ROOT' to your root
# directory of TensorFlow.
#
# Hint: TensorFlow has to be built with it's C++ libraries. For further informations see the
#       official homepage of TensorFlow.
#
# Once done this will be defined:
#
#   TensorFlowCC_FOUND          - TensorFlow C++ libraries found
#   TensorFlowCC_INCLUDE_DIRS   - include directory of the TensorFlow C++ libraries
#   TensorFlowCC_LIBS           - TensorFlow C++ libraries to link against
#

include(FindPackageHandleStandardArgs)

# find 'tensor_interface.h' that is inside include directory
find_path(TensorFlowCC_INTERFACE_DIR
    NAMES
        tensor_interface.h
    PATH_SUFFIXES
        include/tensorflow/c
    PATHS
        ${TensorFlowCC_ROOT}
    REQUIRED
)

set(TensorFlowCC_INCLUDE_DIRS "${TensorFlowCC_INTERFACE_DIR}/../.." CACHE STRING "Include directory to TensorFlow C++ libraries")

unset(TensorFlowCC_INTERFACE_DIR)

find_library(TensorFlow_LIB
    NAMES
        tensorflow
    HINTS
        ${TensorFlowCC_ROOT}
    NO_DEFAULT_PATH
    REQUIRED
)

find_library(TensorFlowCC_LIB
    NAMES
        tensorflow_cc
    HINTS
        ${TensorFlowCC_ROOT}
    NO_DEFAULT_PATH
    REQUIRED
)

find_library(TensorFlowFRAMEWORK_LIB
    NAMES
        tensorflow_framework
    HINTS
        ${TensorFlowCC_ROOT}
    NO_DEFAULT_PATH
    REQUIRED
)

list(APPEND TensorFlowCC_LIBS ${TensorFlow_LIB})
list(APPEND TensorFlowCC_LIBS ${TensorFlowCC_LIB})
list(APPEND TensorFlowCC_LIBS ${TensorFlowFRAMEWORK_LIB})

message(STATUS "Found TensorFlow C++:")
message(STATUS "  Includes: ${TensorFlowCC_INCLUDE_DIRS}")
message(STATUS "  Libraries: ${TensorFlowCC_LIBS}")

set(TensorFlowCC_FOUND TRUE)
