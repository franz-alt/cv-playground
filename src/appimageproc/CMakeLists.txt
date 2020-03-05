set(target imageproc)

message(STATUS "application ${target}")

set(sources
    main.cpp
)

add_executable(${target} ${sources} ${headers})

target_link_libraries(${target}
    PRIVATE
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        png
        libcvpg
        dl
)

set_target_properties(${target}
    PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        PREFIX "cvpg-"
)
