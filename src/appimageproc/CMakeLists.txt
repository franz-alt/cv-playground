set(target imageproc)

message(STATUS "Application '${target}'")

set(sources
    main.cpp
)

add_executable(${target} ${sources} ${headers})

if(BUILD_WITH_TENSORFLOW_SEGMENTATION)
    target_link_libraries(${target}
        PRIVATE
            libcvpg
            ${DEFAULT_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
            ${TensorFlowCC_LIBS}
            ${FFMPEG_LIBRARIES}
            ${Boost_LIBRARIES}
            png
            dl
    )
else()
    target_link_libraries(${target}
        PRIVATE
	    libcvpg
            ${DEFAULT_LIBRARIES}
            ${CMAKE_THREAD_LIBS_INIT}
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
