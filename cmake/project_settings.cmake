# common project header to be used in root project files before or right after the 'project()' command

# set the default build type to release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
