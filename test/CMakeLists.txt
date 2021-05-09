set(target tests)

message(STATUS "Test application '${target}'")

set(sources
    main.cpp
    core/histogram.cpp
    core/image.cpp
    core/meta_data.cpp
    core/multi_array.cpp
    imageproc/algorithms/histogram_equalization.cpp
    imageproc/algorithms/hog.cpp
    imageproc/scripting/convert_to_gray.cpp
    imageproc/scripting/diff.cpp
    imageproc/scripting/image_processor.cpp
    imageproc/scripting/input.cpp
    imageproc/scripting/mean.cpp
    imageproc/scripting/multiply_add.cpp
    imageproc/scripting/scharr.cpp
    imageproc/scripting/sobel.cpp
)

if(BUILD_WITH_FFMPEG)
    list(APPEND sources
        videoproc/packet.cpp
        videoproc/stage_data_handler.cpp
    )
endif()

add_executable(${target} ${sources})

target_link_libraries(${target}
    PRIVATE
        libcvpg
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        gtest_main
        png
        dl
)

if(BUILD_WITH_FFMPEG)
    target_link_libraries(${target} PRIVATE ${FFMPEG_LIBRARIES})
endif()

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    target_link_libraries(${target} PRIVATE ${TensorFlowCC_LIBS})
endif()

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
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
)

add_test(NAME image COMMAND image)
