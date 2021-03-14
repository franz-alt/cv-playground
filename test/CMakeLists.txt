set(target tests)

message(STATUS "Test application '${target}'")

set(sources
    main.cpp
    core/image.cpp
    core/meta_data.cpp
    core/multi_array.cpp
    imageproc/algorithms/histogram_equalization.cpp
    imageproc/scripting/convert_to_gray.cpp
    imageproc/scripting/diff.cpp
    imageproc/scripting/image_processor.cpp
    imageproc/scripting/input.cpp
    imageproc/scripting/mean.cpp
    imageproc/scripting/multiply_add.cpp
    imageproc/scripting/scharr.cpp
    imageproc/scripting/sobel.cpp
    videoproc/packet.cpp
    videoproc/stage_data_handler.cpp
)

add_executable(${target} ${sources})

if(USE_TENSORFLOW_SEGMENTATION)
    target_link_libraries(${target}
        PRIVATE
            ${DEFAULT_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            libcvpg
            ${TensorFlowCC_LIBS}
            ${FFMPEG_LIBRARIES}
            ${Boost_LIBRARIES}
            gtest_main
            png
            dl
    )
else()
    target_link_libraries(${target}
        PRIVATE
            ${DEFAULT_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            libcvpg
            ${FFMPEG_LIBRARIES}
            ${Boost_LIBRARIES}
            gtest_main
            png
            dl
    )
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

add_dependencies(${target} libcvpg)

add_test(NAME image COMMAND image)
