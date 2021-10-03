set(target libcvpg)

message(STATUS "Library '${target}'")

set(headers
    core/exception.hpp
    core/histogram.hpp
    core/image.hpp
    core/meta_data.hpp
    core/multi_array.hpp
    imageproc/algorithms/border_mode.hpp
    imageproc/algorithms/convert_to_gray.hpp
    imageproc/algorithms/convert_to_rgb.hpp
    imageproc/algorithms/histogram_equalization.hpp
    imageproc/algorithms/hog.hpp
    imageproc/algorithms/k_means.hpp
    imageproc/algorithms/otsu_threshold.hpp
    imageproc/algorithms/paint_meta.hpp
    imageproc/algorithms/tiling.hpp
    imageproc/algorithms/tiling/and.hpp
    imageproc/algorithms/tiling/convert_to_gray.hpp
    imageproc/algorithms/tiling/diff.hpp
    imageproc/algorithms/tiling/histogram.hpp
    imageproc/algorithms/tiling/mean.hpp
    imageproc/algorithms/tiling/multiply_add.hpp
    imageproc/algorithms/tiling/or.hpp
    imageproc/algorithms/tiling/parameters.hpp
    imageproc/algorithms/tiling/pooling.hpp
    imageproc/algorithms/tiling/resize.hpp
    imageproc/algorithms/tiling/scharr.hpp
    imageproc/algorithms/tiling/sobel.hpp
    imageproc/algorithms/tiling/threshold.hpp
    imageproc/algorithms/tiling/functors/histogram.hpp
    imageproc/algorithms/tiling/functors/image.hpp
    imageproc/scripting/algorithm_set.hpp
    imageproc/scripting/image_processor.hpp
    imageproc/scripting/item.hpp
    imageproc/scripting/processing_context.hpp
    imageproc/scripting/algorithms/and.hpp
    imageproc/scripting/algorithms/base.hpp
    imageproc/scripting/algorithms/binary_threshold.hpp
    imageproc/scripting/algorithms/convert_to_gray.hpp
    imageproc/scripting/algorithms/convert_to_rgb.hpp
    imageproc/scripting/algorithms/diff.hpp
    imageproc/scripting/algorithms/histogram_equalization.hpp
    imageproc/scripting/algorithms/hog_image.hpp
    imageproc/scripting/algorithms/input.hpp
    imageproc/scripting/algorithms/k_means.hpp
    imageproc/scripting/algorithms/mean.hpp
    imageproc/scripting/algorithms/multiply_add.hpp
    imageproc/scripting/algorithms/or.hpp
    imageproc/scripting/algorithms/paint_meta.hpp
    imageproc/scripting/algorithms/parameter.hpp
    imageproc/scripting/algorithms/parameter_set.hpp
    imageproc/scripting/algorithms/pooling.hpp
    imageproc/scripting/algorithms/resize.hpp
    imageproc/scripting/algorithms/scharr.hpp
    imageproc/scripting/algorithms/sobel.hpp
    imageproc/scripting/algorithms/threshold.hpp
    imageproc/scripting/detail/compiler.hpp
    imageproc/scripting/detail/container_node.hpp
    imageproc/scripting/detail/graph.hpp
    imageproc/scripting/detail/handler.hpp
    imageproc/scripting/detail/node.hpp
    imageproc/scripting/detail/parallel_node.hpp
    imageproc/scripting/detail/parser.hpp
    imageproc/scripting/detail/sequence_node.hpp
    imageproc/scripting/detail/single_node.hpp
    imageproc/scripting/diagnostics/markdown_formatter.hpp
    imageproc/scripting/diagnostics/typedefs.hpp
)

set(sources
    core/exception.cpp
    core/histogram.cpp
    core/image.cpp
    core/meta_data.cpp
    core/multi_array.cpp
    imageproc/algorithms/border_mode.cpp
    imageproc/algorithms/convert_to_gray.cpp
    imageproc/algorithms/convert_to_rgb.cpp
    imageproc/algorithms/histogram_equalization.cpp
    imageproc/algorithms/hog.cpp
    imageproc/algorithms/k_means.cpp
    imageproc/algorithms/otsu_threshold.cpp
    imageproc/algorithms/paint_meta.cpp
    imageproc/algorithms/tiling/and.cpp
    imageproc/algorithms/tiling/convert_to_gray.cpp
    imageproc/algorithms/tiling/diff.cpp
    imageproc/algorithms/tiling/histogram.cpp
    imageproc/algorithms/tiling/mean.cpp
    imageproc/algorithms/tiling/multiply_add.cpp
    imageproc/algorithms/tiling/or.cpp
    imageproc/algorithms/tiling/pooling.cpp
    imageproc/algorithms/tiling/resize.cpp
    imageproc/algorithms/tiling/scharr.cpp
    imageproc/algorithms/tiling/sobel.cpp
    imageproc/algorithms/tiling/threshold.cpp
    imageproc/scripting/algorithm_set.cpp
    imageproc/scripting/image_processor.cpp
    imageproc/scripting/item.cpp
    imageproc/scripting/processing_context.cpp
    imageproc/scripting/algorithms/and.cpp
    imageproc/scripting/algorithms/binary_threshold.cpp
    imageproc/scripting/algorithms/convert_to_gray.cpp
    imageproc/scripting/algorithms/convert_to_rgb.cpp
    imageproc/scripting/algorithms/diff.cpp
    imageproc/scripting/algorithms/histogram_equalization.cpp
    imageproc/scripting/algorithms/hog_image.cpp
    imageproc/scripting/algorithms/input.cpp
    imageproc/scripting/algorithms/k_means.cpp
    imageproc/scripting/algorithms/mean.cpp
    imageproc/scripting/algorithms/multiply_add.cpp
    imageproc/scripting/algorithms/or.cpp
    imageproc/scripting/algorithms/paint_meta.cpp
    imageproc/scripting/algorithms/parameter.cpp
    imageproc/scripting/algorithms/parameter_set.cpp
    imageproc/scripting/algorithms/pooling.cpp
    imageproc/scripting/algorithms/resize.cpp
    imageproc/scripting/algorithms/scharr.cpp
    imageproc/scripting/algorithms/sobel.cpp
    imageproc/scripting/algorithms/threshold.cpp
    imageproc/scripting/detail/compiler.cpp
    imageproc/scripting/detail/container_node.cpp
    imageproc/scripting/detail/graph.cpp
    imageproc/scripting/detail/handler.cpp
    imageproc/scripting/detail/parallel_node.cpp
    imageproc/scripting/detail/parser.cpp
    imageproc/scripting/detail/sequence_node.cpp
    imageproc/scripting/detail/single_node.cpp
)

if(BUILD_WITH_FFMPEG)
    list(APPEND headers
        videoproc/any_stage.hpp
        videoproc/frame.hpp
        videoproc/packet.hpp
        videoproc/stage_data_handler.hpp
        videoproc/stage_parameters.hpp
        videoproc/update_indicator.hpp
        videoproc/pipelines/any_pipeline.hpp
        videoproc/pipelines/file_to_file.hpp
        videoproc/pipelines/parameters.hpp
        videoproc/pipelines/rtsp_to_file.hpp
        videoproc/processors/frame.hpp
        videoproc/processors/interframe.hpp
        videoproc/sinks/file.hpp
        videoproc/sources/file.hpp
        videoproc/sources/rtsp.hpp
    )

    list(APPEND sources
        videoproc/frame.cpp
        videoproc/packet.cpp
        videoproc/stage_data_handler.cpp
        videoproc/update_indicator.cpp
        videoproc/pipelines/file_to_file.cpp
        videoproc/pipelines/rtsp_to_file.cpp
        videoproc/processors/frame.cpp
        videoproc/processors/interframe.cpp
        videoproc/sinks/file.cpp
        videoproc/sources/file.cpp
        videoproc/sources/rtsp.cpp
    )
endif()

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    list(APPEND headers
        imageproc/algorithms/tfpredict.hpp
        imageproc/scripting/algorithms/tfpredict.hpp
    )

    list(APPEND sources
        imageproc/algorithms/tfpredict.cpp
        imageproc/scripting/algorithms/tfpredict.cpp
    )
endif()

add_library(${target} ${sources} ${headers})

target_include_directories(${target}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

if(BUILD_WITH_FFMPEG)
    target_include_directories(${target}
        PRIVATE
            $<BUILD_INTERFACE:${FFMPEG_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${SWSCALE_INCLUDE_DIRS}>
    )
endif()

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${TensorFlowCC_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${TensorFlowCC_INCLUDE_DIRS}/src>
    )
endif()

target_compile_definitions(${target}
    PUBLIC
        ${DEFAULT_COMPILE_DEFINITIONS}
        $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:${target}_STATIC_DEFINE>
)

if(LIST_OF_SANITIZERS)
    if(NOT "${LIST_OF_SANITIZERS}" STREQUAL "")
        target_compile_options(${target}
                               INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
        target_link_libraries(${target}
                              INTERFACE -fsanitize=${LIST_OF_SANITIZERS})
    endif()
endif()

set_target_properties(${target}
    PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        PREFIX ""
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
        VERSION ${CVPG_VERSION}
        SOVERSION ${CVPG_VERSION_MAJOR}
)

# install library
install(TARGETS ${target} DESTINATION lib)

# install headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/"
        DESTINATION "${INSTALL_INCLUDE_DIR}/${target}"
        FILES_MATCHING
        PATTERN "*.hpp")

# define GNU standard installation directories
include(GNUInstallDirs)

# set some directories and files
set(CONFIG_INSTALL_DIR  "${CMAKE_INSTALL_LIBDIR}/cmake/${target}")

set(GENERATED_DIR       "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(VERSION_CONFIG_FILE "${GENERATED_DIR}/${target}ConfigVersion.cmake")
set(PROJECT_CONFIG_FILE "${GENERATED_DIR}/${target}Config.cmake")
set(TARGETS_CONFIG_FILE "${GENERATED_DIR}/${target}Targets.cmake")

# use helpers functions for creating config files that can be included by other projects to find and use a package.
include(CMakePackageConfigHelpers)

# create version file for project
write_basic_package_version_file(
    "${VERSION_CONFIG_FILE}"
    VERSION "${CVPG_VERSION}"
    COMPATIBILITY SameMajorVersion
)

# create configuration file for project
configure_package_config_file(
    "${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${PROJECT_CONFIG_FILE}" INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)

# export project targets to configuration file
export(
    TARGETS ${target}
    FILE "${TARGETS_CONFIG_FILE}"
)

# install configuration files to installation directory
install(
    FILES
        ${PROJECT_CONFIG_FILE}
        ${VERSION_CONFIG_FILE}
    DESTINATION
        ${CONFIG_INSTALL_DIR}
)
