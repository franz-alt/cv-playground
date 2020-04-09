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

add_test(NAME image COMMAND image)
