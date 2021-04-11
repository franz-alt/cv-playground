# cv-playground

Library containing some easy and scalable computer vision algorithms

## Motivation

At a private video surveillance project some computer vision algorithms are needed. One goal was not to use an existing computer vision library like OpenCV or Halcon. Writing algorithms from scratch, learning how to make a library for these algorithms and creating some funny examples leads to the *cv-playground* library.

## Algorithms

At the moment only some easy algorithms at different categories are implemented.

### Arithmetic

* Difference
* Multiply Add (*Scaling*)

### Edge Detection

* Scharr
* Sobel

### Image Enhancement

* Histogram Equalization

### Miscellaneous

* Paint Primitives

### Object Detection

* TensorFlow Inferencing (see section *Options*)

### Smoothing

* Mean

## Sample Applications

### imageproc

An application used to apply multiple filters stored at a script file to an image.

### videoproc

An application used to apply multiple filters stored at script files to all frames and inter-frames of a MP4 video file or a RTSP video stream.

## Planned

* Vectorized (AVX 2) algorithms

## Building

### Requirements

* C++ 17 compiler
* CMake 3.12 or later
* TensorFlow 2 or later (see section *Options*)

### Options

The following options could be used to build the library, sample applications and/or unit tests.

    option(BUILD_APPS "Build applications." ON)
    option(BUILD_SHARED_LIBS "Build shared instead of static libraries." ON)
    option(BUILD_TESTS "Build module tests." ON)

Advanced debugging could be achived with so called [sanitizers](https://hpc-wiki.info/hpc/Compiler_Sanitizers) built inside modern compilers.
    
    option(BUILD_WITH_SANITIZER_ADDRESS "Build all libraries and applications with address sanitizer." OFF)
    option(BUILD_WITH_SANITIZER_MEMORY "Build all libraries and applications with memory sanitizer." OFF)
    option(BUILD_WITH_SANITIZER_UNDEFINED_BEHAVIOUR "Build all libraries and applications with undefined behaviour sanitizer." OFF)

Object detection with TensorFlow could be performed with the *tfpredict* algorithm. To enable this feature you have to edit the following option

    option(USE_TENSORFLOW_SEGMENTATION "Use Google TensorFlow for image segmentation." OFF)

and set the environment variable *TensorFlowCC_ROOT*.

### Example

#### Build At Console

To build the library on Unix/Linux try:

    mkdir build
    cd build
    cmake ..
    make -j`nproc`

#### Build With Docker

At the [docker](docker) subdirectory several Dockerfiles are available to build the library based on different Linux distributions.

To build a Docker image based on Debian Buster type:

    docker build -f Dockerfile.debian-buster --force-rm -t libcvpg:0.1 .

inside the [docker](docker) subdirectory.

To create a Docker container and execute a bash script inside the container type:

    docker run -it libcvpg:0.1 /bin/bash

## License

Copyright (c) 2020 Franz Alt

This project is distributed under the [MIT License](https://opensource.org/licenses/MIT), see [LICENSE](./LICENSE) for more information.
