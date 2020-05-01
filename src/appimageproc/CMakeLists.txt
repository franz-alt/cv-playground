include(${CMAKE_SOURCE_DIR}/cmake/instruction_set.cmake)

set(target imageproc)

message(STATUS "Application '${target}'")

set(sources
    main.cpp
)

add_executable(${target} ${sources} ${headers})
cvpg_target_instruction_set_compile_options(${target})

target_link_libraries(${target}
    PRIVATE
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${Boost_LIBRARIES}
        png
        libcvpg
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
