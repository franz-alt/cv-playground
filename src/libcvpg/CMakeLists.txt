set(target libcvpg)

message(STATUS "library ${target}")

set(headers
    core/exception.hpp
    core/image.hpp
    imageproc/tiling.hpp
    imageproc/tiling/diff.hpp
    imageproc/tiling/mean.hpp
    imageproc/tiling/multiply_add.hpp
)

set(sources
    core/exception.cpp
    core/image.cpp
    imageproc/tiling.cpp
    imageproc/tiling/diff.cpp
    imageproc/tiling/mean.cpp
    imageproc/tiling/multiply_add.cpp
)

add_library(${target} ${sources} ${headers})

target_include_directories(${target}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_compile_definitions(${target}
    PUBLIC
        ${DEFAULT_COMPILE_DEFINITIONS}
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target}_STATIC_DEFINE>
)

set_target_properties(${target}
    PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        PREFIX ""
        VERSION ${CVPG_VERSION}
        SOVERSION ${CVPG_VERSION_MAJOR}
)
