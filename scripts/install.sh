#!/bin/bash

PACKAGES="
    build-essential
    ccache
    libglu1-mesa-dev
    uuid-dev
    libgtk-3-dev
    libasound2-dev
    libopenal-dev
    libpulse-dev
    libvulkan1
    mesa-vulkan-drivers
    vulkan-utils
    libgles2-mesa-dev
"

apt-get install $PACKAGES
