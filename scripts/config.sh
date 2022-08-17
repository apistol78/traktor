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
export RPI_SYSROOT=$TRAKTOR_3RDP/raspberry-sysroot
export MOLTEN_SDK=$TRAKTOR_3RDP/vulkan/vulkansdk-macos-1.2.154.0/MoltenVK 

export AGG_SDK=$TRAKTOR_HOME/3rdp_/agg-2.6
export ASTCENC_SDK=$TRAKTOR_HOME/3rdp_/astc-encoder
export BULLET_SDK=$TRAKTOR_HOME/3rdp_/bullet3
export EXPAT_SDK=$TRAKTOR_HOME/3rdp_/libexpat/expat
export FBX_SDK=$TRAKTOR_HOME/3rdp_/fbx/2016.1.2
export FLAC_SDK=$TRAKTOR_HOME/3rdp_/flac
export FREETYPE_SDK=$TRAKTOR_HOME/3rdp_/freetype
export JPEG_SDK=$TRAKTOR_HOME/3rdp_/jpeg/jpeg-9e
export LUA_SDK=$TRAKTOR_HOME/3rdp_/lua/lua-5.4.4
export LZF_SDK=$TRAKTOR_HOME/3rdp_/liblzf/liblzf-3.6
export LZO_SDK=$TRAKTOR_HOME/3rdp_/lzo/lzo-2.10
export MIKKTSPACE_SDK=$TRAKTOR_HOME/3rdp_/MikkTSpace
export OGG_SDK=$TRAKTOR_HOME/3rdp_/ogg
export P4_SDK=$TRAKTOR_HOME/3rdp_/p4api-linux/p4api-2022.1.2305383
export PNG_SDK=$TRAKTOR_HOME/3rdp_/libpng
export RAPIDJSON_SDK=$TRAKTOR_HOME/3rdp_/rapidjson
export RECAST_SDK=$TRAKTOR_HOME/3rdp_/recastnavigation
export RG_ETC1_SDK=$TRAKTOR_HOME/3rdp_/rg-etc1
export STB_SDK=$TRAKTOR_HOME/3rdp_/stb
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp_/steamworks/sdk
export SQLITE_SDK=$TRAKTOR_HOME/3rdp_/sqlite
export SQUISH_SDK=$TRAKTOR_HOME/3rdp_/squish
export THEORA_SDK=$TRAKTOR_HOME/3rdp_/theora
export TINYEXR_SDK=$TRAKTOR_HOME/3rdp_/tinyexr
export VORBIS_SDK=$TRAKTOR_HOME/3rdp_/vorbis
export XATLAS_SDK=$TRAKTOR_HOME/3rdp_/xatlas
export ZLIB_SDK=$TRAKTOR_HOME/3rdp_/zlib

# Vulkan SDK
# - cleanup Extern and add Extern.vulkan which contain all paths,
# and replace in Traktor.Vulkan and Traktor.Vulkan.Editor
# VULKAN_SDK
#	VULKAN_HEADERS_SDK
#	VULKAN_VMA_SDK
#	VULKAN_GLSLANG_SDK
#	VULKAN_SPIRVTOOLS_SDK
#	VULKAN_SPIRVCROSS_SDK
export VULKAN_SDK=$TRAKTOR_HOME/3rdp_/vulkan-linux/1.3.216.0/source/Vulkan-Headers
export VMA_SDK=$TRAKTOR_HOME/3rdp_/vulkan-linux/1.3.216.0/source/VulkanMemoryAllocator
export GLSLANG_SDK=$TRAKTOR_HOME/3rdp_/vulkan-linux/1.3.216.0/source/glslang
export SPIRVTOOLS_SDK=$TRAKTOR_HOME/3rdp_/vulkan-linux/1.3.216.0/source/SPIRV-Tools
export SPIRVCROSS_SDK=$TRAKTOR_HOME/3rdp_/vulkan-linux/1.3.216.0/source/SPIRV-Cross

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
	#export EMBREE_SDK=$TRAKTOR_3RDP/embree-3.5.2.x86_64.linux
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp_/embree-linux/embree-3.13.4.x86_64.linux
	#export OIDN_SDK=$TRAKTOR_3RDP/oidn-1.3.0.x86_64.linux
	export OIDN_SDK=$TRAKTOR_HOME/3rdp_/oidn-linux/oidn-1.4.3.x86_64.linux
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	#export EMBREE_SDK=$TRAKTOR_3RDP/embree-3.6.1.x86_64.macosx
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp_/embree-macOS/embree-3.13.4.x86_64.macosx
	#export OIDN_SDK=$TRAKTOR_3RDP/oidn-1.3.0.x86_64.macos
	export OIDN_SDK=$TRAKTOR_HOME/3rdp_/oidn-macOS/oidn-1.4.3.x86_64.macos
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
