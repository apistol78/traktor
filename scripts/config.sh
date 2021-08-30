#!/bin/bash

# Get normalized path to this script, excluding file name.
TRAKTOR_HOME="`dirname \"$BASH_SOURCE\"`/.."
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"
export TRAKTOR_HOME

# Paths to 3rd party dependencies.
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp/steamworks-sdk-142
export BULLET_SDK=$TRAKTOR_HOME/3rdp/bullet3-master-210317
export FBX_SDK=$TRAKTOR_HOME/3rdp/FbxSdk/2016.1.2
export GLSL_OPTIMIZER=$TRAKTOR_HOME/3rdp/glsl-optimizer-master-20141020
export GLSLANG=$TRAKTOR_HOME/3rdp/glslang-master-200605
export SQLITE_SDK=$TRAKTOR_HOME/3rdp/sqlite-3.8.7.2
export LUA_SDK=$TRAKTOR_HOME/3rdp/lua-5.4.3
export GLEW_SDK=$TRAKTOR_HOME/3rdp/glew-1.13.0
export EXPAT_SDK=$TRAKTOR_HOME/3rdp/libexpat-R_2_2_5/expat
export ZLIB_SDK=$TRAKTOR_HOME/3rdp/zlib-1.2.11
export P4_SDK=$TRAKTOR_HOME/3rdp/p4api-2018.1.1660568
export RPI_SYSROOT=$TRAKTOR_HOME/3rdp/raspberry-sysroot
export VULKAN_SDK=$TRAKTOR_HOME/3rdp/vulkan/Vulkan-Headers-master-201012
export MOLTEN_SDK=$TRAKTOR_HOME/3rdp/vulkan/vulkansdk-macos-1.2.154.0/MoltenVK 
export VMA_SDK=$TRAKTOR_HOME/3rdp/VulkanMemoryAllocator-master-210504
export SPIRVTOOLS_SDK=$TRAKTOR_HOME/3rdp/SPIRV-Tools-master-201029
export SPIRVCROSS_SDK=$TRAKTOR_HOME/3rdp/SPIRV-Cross-master-201009
export ASTCENC_SDK=$TRAKTOR_HOME/3rdp/astc-encoder-master-201104

# Determine platform from some simple heuristic.
if [[ `uname -s` == Linux* ]]; then
	if [[ `uname -m` == arm* ]]; then
		export TRAKTOR_PLATFORM=rpi
	else
		export TRAKTOR_PLATFORM=linux
	fi
elif [[ `uname -s` == Darwin* ]]; then
	export TRAKTOR_PLATFORM=macos
fi

# Platform specific 3rd party dependencies.
if [[ $TRAKTOR_PLATFORM == linux ]]; then
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp/embree-3.5.2.x86_64.linux
	export OIDN_SDK=$TRAKTOR_HOME/3rdp/oidn-1.3.0.x86_64.linux
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp/embree-3.6.1.x86_64.macosx
	export OIDN_SDK=$TRAKTOR_HOME/3rdp/oidn-1.3.0.x86_64.macos
fi

# Export name of solution builder binary.
if [[ $TRAKTOR_PLATFORM == linux ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/linux/releasestatic/Traktor.SolutionBuilder.App"
	export RUN="$TRAKTOR_HOME/bin/linux/releasestatic/Traktor.Run.App"
elif [[ $TRAKTOR_PLATFORM == rpi ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/rpi/releasestatic/Traktor.SolutionBuilder.App"
	export RUN="$TRAKTOR_HOME/bin/rpi/releasestatic/Traktor.Run.App"
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	export SOLUTIONBUILDER="$TRAKTOR_HOME/bin/osx/releasestatic/Traktor.SolutionBuilder.App"
	export RUN="$TRAKTOR_HOME/bin/osx/releasestatic/Traktor.Run.App"
fi
