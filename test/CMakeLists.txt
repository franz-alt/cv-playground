set(target tests)

message(STATUS "${target}")

set(sources
    main.cpp
    core/image.cpp
)

add_executable(${target} ${sources})

target_link_libraries(${target}
    PRIVATE
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        gtest_main
        png
        libcvpg
)

add_test(NAME image COMMAND image)
