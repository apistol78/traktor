#!/bin/bash

# add lunarg vulkan repository.
wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.1.101-bionic.list http://packages.lunarg.com/vulkan/1.1.101/lunarg-vulkan-1.1.101-bionic.list
apt update

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
    lunarg-vulkan-sdk
    mesa-vulkan-drivers
    libgles2-mesa-dev
    libopenimageio-dev
    libglew-dev
    freeglut3-dev
    opencl-dev
"

apt-get install $PACKAGES
apt install lunarg-vulkan-sdk