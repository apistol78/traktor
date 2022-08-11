#!/bin/bash

# Get normalized path to this script, excluding file name.
if [[ $BASH_SOURCE != "" ]]; then
	SCRIPT=$BASH_SOURCE
else
	SCRIPT=$0
fi
TRAKTOR_HOME="`dirname \"$SCRIPT\"`/.."
TRAKTOR_HOME="`(cd \"$TRAKTOR_HOME\" && pwd)`"
export TRAKTOR_HOME

# Path to 3rd party packages.
export TRAKTOR_3RDP=$TRAKTOR_HOME/3rdp

# Paths to 3rd party dependencies.
export STEAMWORKS_SDK=$TRAKTOR_3RDP/steamworks-sdk-142
export BULLET_SDK=$TRAKTOR_3RDP/bullet3-master-210317
export FBX_SDK=$TRAKTOR_3RDP/FbxSdk/2016.1.2
export GLSLANG=$TRAKTOR_3RDP/glslang-master-200605
export SQLITE_SDK=$TRAKTOR_3RDP/sqlite-3.8.7.2
export LUA_SDK=$TRAKTOR_3RDP/lua-5.4.3
export EXPAT_SDK=$TRAKTOR_3RDP/libexpat-R_2_2_5/expat
export ZLIB_SDK=$TRAKTOR_3RDP/zlib-1.2.11
export P4_SDK=$TRAKTOR_3RDP/p4api-2018.1.1660568
export RPI_SYSROOT=$TRAKTOR_3RDP/raspberry-sysroot
export VULKAN_SDK=$TRAKTOR_3RDP/vulkan/Vulkan-Headers-master-201012
export MOLTEN_SDK=$TRAKTOR_3RDP/vulkan/vulkansdk-macos-1.2.154.0/MoltenVK 
export VMA_SDK=$TRAKTOR_3RDP/VulkanMemoryAllocator-master-210504
export SPIRVTOOLS_SDK=$TRAKTOR_3RDP/SPIRV-Tools-master-201029
export SPIRVCROSS_SDK=$TRAKTOR_3RDP/SPIRV-Cross-master-201009
export ASTCENC_SDK=$TRAKTOR_3RDP/astc-encoder-master-201104

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
	export EMBREE_SDK=$TRAKTOR_3RDP/embree-3.5.2.x86_64.linux
	export OIDN_SDK=$TRAKTOR_3RDP/oidn-1.3.0.x86_64.linux
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	export EMBREE_SDK=$TRAKTOR_3RDP/embree-3.6.1.x86_64.macosx
	export OIDN_SDK=$TRAKTOR_3RDP/oidn-1.3.0.x86_64.macos
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
