set(target tests)

message(STATUS "${target}")

set(sources
    main.cpp
    core/image.cpp
    imageproc/algorithms/histogram_equalization.cpp
    imageproc/scripting/image_processor.cpp
)

add_executable(${target} ${sources})

target_link_libraries(${target}
    PRIVATE
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        gtest_main
        png
        libcvpg
        dl
)

set_target_properties(${target}
    PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
)

add_test(NAME image COMMAND image)
