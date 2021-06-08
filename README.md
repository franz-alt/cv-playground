# cv-playground

Library containing some easy and scalable computer vision algorithms

## Motivation

At a private video surveillance project some computer vision algorithms are needed. One goal was not to use an existing computer vision library like OpenCV or Halcon. Writing algorithms from scratch, learning how to make a library for these algorithms and creating some funny examples leads to the *cv-playground* library.

## Algorithms

At the moment only some easy algorithms at different categories are implemented.

### Arithmetic

* And
* Difference
* Multiply Add (*Scaling*)
* Or

### Edge Detection

* Scharr
* Sobel

### Image Enhancement

* Histogram Equalization

### Geometric Transformations

* Pooling
* Resize
* Resize To Target

### Miscellaneous

* Paint Primitives

### Object Detection

* Histogram Of Oriented Gradients [Experimental]
* TensorFlow Inferencing (see section *Options*) [Experimental]

### Segmentation

* Binary Threshold
* K-Means [Experimental]
* Threshold

### Smoothing

* Mean

## Sample Applications

### imageproc

An application used to apply multiple filters stored at a script file to an image.

### videoproc

An application used to apply multiple filters stored at script files to all frames and inter-frames of a MP4 video file or a RTSP video stream.

Hint: This application is only available if FFmpeg support is enabled.

## Planned

* Vectorized (AVX-2) algorithms

## Building

### Requirements

* C++ 17 compiler
* CMake 3.12 or later
* FFmpeg 4.x or later
* TensorFlow 2 or later (see section *Options* and TensorFlow Support at *doc* subdirectory)

### Options

The following options could be used to build the library, sample applications and/or unit tests.

    option(BUILD_APPS "Build applications." ON)
    option(BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
    option(BUILD_TESTS "Build module tests." ON)
    option(BUILD_WITH_TCMALLOC "Build the software with TCMalloc." OFF)
    option(BUILD_WITH_FFMPEG "Build with enabled FFmpeg support for video processing." ON)

Advanced debugging could be achived with so called [sanitizers](https://hpc-wiki.info/hpc/Compiler_Sanitizers) built inside modern compilers.
    
    option(BUILD_WITH_SANITIZER_ADDRESS "Build all libraries and applications with address sanitizer." OFF)
    option(BUILD_WITH_SANITIZER_MEMORY "Build all libraries and applications with memory sanitizer." OFF)
    option(BUILD_WITH_SANITIZER_UNDEFINED_BEHAVIOUR "Build all libraries and applications with undefined behaviour sanitizer." OFF)

### Example

#### Build At Console

To build the project on Unix/Linux try:

    mkdir build
    cd build
    cmake ..
    make -j`nproc`

#### Build With Docker

At the [docker](docker) subdirectory several Dockerfiles are available to build the project based on different Linux distributions.

For instance, to build a Docker image based on Debian Ubuntu 20.04 you have to execute

    docker build -f Dockerfile.ubuntu-20.04 -t cv-playground/dev:1.0 .

inside the [docker](docker) subdirectory.

To build the project inside the Docker container you have to run the Docker image and mount your project root directory from host side to /usr/src/cv-playground or another directory with the command

    docker run -v /home/user/cv-playground:/usr/src/cv-playground --rm -it cv-playground/dev:1.0 /bin/bash

At this point your could build the project as described at 'Build At Console'.

#### Build With Docker And TensorFlow Support

To build the project with TensorFlow support read the [TensorFlow](./doc/TensorFlowSupport.md) section.

## License

Copyright (c) 2020-2021 Franz Alt

This project is distributed under the [MIT License](https://opensource.org/licenses/MIT), see [LICENSE](./LICENSE) for more information.
