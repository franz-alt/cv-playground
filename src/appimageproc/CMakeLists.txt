set(target imageproc)

message(STATUS "Application '${target}'")

set(sources
    main.cpp
)

add_executable(${target} ${sources} ${headers})

target_link_libraries(${target}
    PRIVATE
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${FFMPEG_LIBRARIES}
        ${Boost_LIBRARIES}
        libcvpg
        png
        dl
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
        PREFIX "cvpg-"
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
)

# install application
install(TARGETS ${target} DESTINATION bin)
