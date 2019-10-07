#!/bin/bash

# Get normalized path to this script, excluding file name.
TRAKTOR_HOME="`dirname \"$BASH_SOURCE\"`/.."
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"
export TRAKTOR_HOME

# Paths to 3rd party dependencies.
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp/steamworks-sdk-142
export BULLET_SDK=$TRAKTOR_HOME/3rdp/bullet3-master-190726
export FBX_SDK=$TRAKTOR_HOME/3rdp/FbxSdk/2016.1.2
export GLSL_OPTIMIZER=$TRAKTOR_HOME/3rdp/glsl-optimizer-master-20141020
export GLSLANG=$TRAKTOR_HOME/3rdp/glslang-master-190407
export SQLITE_SDK=$TRAKTOR_HOME/3rdp/sqlite-3.8.7.2
export LUA_SDK=$TRAKTOR_HOME/3rdp/lua-5.3.5
export GLEW_SDK=$TRAKTOR_HOME/3rdp/glew-1.13.0
export EXPAT_SDK=$TRAKTOR_HOME/3rdp/libexpat-R_2_2_5/expat
export ZLIB_SDK=$TRAKTOR_HOME/3rdp/zlib-1.2.11
export P4_SDK=$TRAKTOR_HOME/3rdp/p4api-2018.1.1660568
export RPI_SYSROOT=$TRAKTOR_HOME/3rdp/raspberry-sysroot
export VULKAN_SDK=$TRAKTOR_HOME/3rdp/vulkan/Vulkan-Headers-master-190605
export VMA_SDK=$TRAKTOR_HOME/3rdp/VulkanMemoryAllocator-master
export ASTC_SDK=$TRAKTOR_HOME/3rdp/astc-encoder-master-190814

# Platform specific 3rd party dependencies.
if [[ `uname -s` == Linux* ]]; then
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp/embree-3.5.2.x86_64.linux
	export OIDN_SDK=$TRAKTOR_HOME/3rdp/oidn-1.0.0.x86_64.linux
elif [[ `uname -s` == Darwin* ]]; then
	export EMBREE_SDK=
	export OIDN_SDK=$TRAKTOR_HOME/3rdp/oidn-1.0.0.x86_64.macos
fi

# Export name of solution builder binary.
if [[ `uname -s` == Linux* ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/linux/releasestatic/Traktor.SolutionBuilder.App"
elif [[ `uname -s` == Darwin* ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/osx/releasestatic/Traktor.SolutionBuilder.App"
fi
