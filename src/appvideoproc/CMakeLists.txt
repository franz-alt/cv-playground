set(target videoproc)

message(STATUS "Application '${target}'")

set(headers
    progress_monitor.hpp
)

set(sources
    main.cpp
    progress_monitor.cpp
)

add_executable(${target} ${sources} ${headers})

target_link_libraries(${target}
    PRIVATE
        libcvpg
        ${DEFAULT_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        ${FFMPEG_LIBRARIES}
        ${SWSCALE_LIBRARIES}
        ${Boost_LIBRARIES}
        png
        dl
)

if(BUILD_WITH_TCMALLOC)
    target_link_libraries(${target}
        PRIVATE
            ${TCMalloc_LIBRARIES}
    )
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
        PREFIX "cvpg-"
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
)

# install application
install(TARGETS ${target} DESTINATION bin)
