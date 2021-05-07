set(target videoproc)

message(STATUS "Application '${target}'")

find_package(FFmpeg REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL)

include_directories(${FFMPEG_INCLUDE_DIRS})

set(headers
    progress_monitor.hpp
)

set(sources
    main.cpp
    progress_monitor.cpp
)

add_executable(${target} ${sources} ${headers})

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    target_link_libraries(${target}
        PRIVATE
            ${DEFAULT_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            libcvpg
            ${TensorFlowCC_LIBS}
            ${FFMPEG_LIBRARIES}
            ${Boost_LIBRARIES}
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
        PREFIX "cvpg-"
        ${BUILDTYPE}_POSTFIX "${TARGET_PREFIX}"
)

add_dependencies(${target} libcvpg)

# install application
install(TARGETS ${target} DESTINATION bin)
