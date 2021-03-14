set(target libcvpg)

message(STATUS "Library '${target}'")

find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL)

include_directories(${FFMPEG_INCLUDE_DIRS})

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
    imageproc/algorithms/tiling.hpp
    imageproc/algorithms/tiling/convert_to_gray.hpp
    imageproc/algorithms/tiling/diff.hpp
    imageproc/algorithms/tiling/histogram.hpp
    imageproc/algorithms/tiling/mean.hpp
    imageproc/algorithms/tiling/multiply_add.hpp
    imageproc/algorithms/tiling/parameters.hpp
    imageproc/algorithms/tiling/scharr.hpp
    imageproc/algorithms/tiling/sobel.hpp
    imageproc/algorithms/tiling/functors/histogram.hpp
    imageproc/algorithms/tiling/functors/image.hpp
    imageproc/scripting/algorithm_set.hpp
    imageproc/scripting/image_processor.hpp
    imageproc/scripting/item.hpp
    imageproc/scripting/processing_context.hpp
    imageproc/scripting/algorithms/base.hpp
    imageproc/scripting/algorithms/convert_to_gray.hpp
    imageproc/scripting/algorithms/convert_to_rgb.hpp
    imageproc/scripting/algorithms/diff.hpp
    imageproc/scripting/algorithms/histogram_equalization.hpp
    imageproc/scripting/algorithms/input.hpp
    imageproc/scripting/algorithms/mean.hpp
    imageproc/scripting/algorithms/multiply_add.hpp
    imageproc/scripting/algorithms/parameter.hpp
    imageproc/scripting/algorithms/parameter_set.hpp
    imageproc/scripting/algorithms/scharr.hpp
    imageproc/scripting/algorithms/sobel.hpp
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
    videoproc/any_stage.hpp
    videoproc/frame.hpp
    videoproc/packet.hpp
    videoproc/stage_data_handler.hpp
    videoproc/stage_fsm.hpp
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
    imageproc/algorithms/tiling/convert_to_gray.cpp
    imageproc/algorithms/tiling/diff.cpp
    imageproc/algorithms/tiling/histogram.cpp
    imageproc/algorithms/tiling/mean.cpp
    imageproc/algorithms/tiling/multiply_add.cpp
    imageproc/algorithms/tiling/scharr.cpp
    imageproc/algorithms/tiling/sobel.cpp
    imageproc/scripting/algorithm_set.cpp
    imageproc/scripting/image_processor.cpp
    imageproc/scripting/item.cpp
    imageproc/scripting/processing_context.cpp
    imageproc/scripting/algorithms/convert_to_gray.cpp
    imageproc/scripting/algorithms/convert_to_rgb.cpp
    imageproc/scripting/algorithms/diff.cpp
    imageproc/scripting/algorithms/histogram_equalization.cpp
    imageproc/scripting/algorithms/input.cpp
    imageproc/scripting/algorithms/mean.cpp
    imageproc/scripting/algorithms/multiply_add.cpp
    imageproc/scripting/algorithms/parameter.cpp
    imageproc/scripting/algorithms/parameter_set.cpp
    imageproc/scripting/algorithms/scharr.cpp
    imageproc/scripting/algorithms/sobel.cpp
    imageproc/scripting/detail/compiler.cpp
    imageproc/scripting/detail/container_node.cpp
    imageproc/scripting/detail/graph.cpp
    imageproc/scripting/detail/handler.cpp
    imageproc/scripting/detail/parallel_node.cpp
    imageproc/scripting/detail/parser.cpp
    imageproc/scripting/detail/sequence_node.cpp
    imageproc/scripting/detail/single_node.cpp
    videoproc/frame.cpp
    videoproc/packet.cpp
    videoproc/stage_data_handler.cpp
    videoproc/stage_fsm.cpp
    videoproc/update_indicator.cpp
    videoproc/pipelines/file_to_file.cpp
    videoproc/pipelines/rtsp_to_file.cpp
    videoproc/processors/frame.cpp
    videoproc/processors/interframe.cpp
    videoproc/sinks/file.cpp
    videoproc/sources/file.cpp
    videoproc/sources/rtsp.cpp
)

if(USE_TENSORFLOW_SEGMENTATION)
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

if(USE_TENSORFLOW_SEGMENTATION)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
            $<BUILD_INTERFACE:${TensorFlowCC_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${TensorFlowCC_INCLUDE_DIRS}/src>
    )
else()
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
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
