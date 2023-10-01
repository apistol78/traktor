#!/bin/bash

# install packages.
PACKAGES="
    build-essential
    ccache
    libglu1-mesa-dev
    uuid-dev
    libgtk-3-dev
    libasound2-dev
    libopenal-dev
    libpulse-dev
    mesa-vulkan-drivers
    clang
    lld
    gdb
"

apt-get install $PACKAGES

