#!/bin/bash

# install packages.
PACKAGES="
    clang
    lld
    build-essential
    ccache
    libglu1-mesa-dev
    uuid-dev
    libgtk-3-dev
    libasound2-dev
    libopenal-dev
    libpulse-dev
    mesa-vulkan-drivers
    libgles2-mesa-dev
    libopenimageio-dev
    libglew-dev
    freeglut3-dev
    opencl-dev
"

apt-get install $PACKAGES

