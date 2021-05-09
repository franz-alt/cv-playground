if(BUILD_APPS)
    add_subdirectory(appimageproc)

    if(BUILD_WITH_FFMPEG)
        add_subdirectory(appvideoproc)
    endif()
endif()

add_subdirectory(libcvpg)
