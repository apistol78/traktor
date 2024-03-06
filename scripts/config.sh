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

# Paths to 3rd party dependencies.
export RPI_SYSROOT=$TRAKTOR_HOME/3rdp-restricted/raspberry-sysroot
export MOLTEN_SDK=$TRAKTOR_HOME/3rdp-restricted/vulkan/vulkansdk-macos-1.2.154.0/MoltenVK 

export AGG_SDK=$TRAKTOR_HOME/3rdp/agg-2.6
export ASTCENC_SDK=$TRAKTOR_HOME/3rdp/astc-encoder
export BULLET_SDK=$TRAKTOR_HOME/3rdp/bullet3
export EXPAT_SDK=$TRAKTOR_HOME/3rdp/libexpat/expat
export FBX_SDK=$TRAKTOR_HOME/3rdp/fbx
export FLAC_SDK=$TRAKTOR_HOME/3rdp/flac
export FREETYPE_SDK=$TRAKTOR_HOME/3rdp/freetype
export JPEG_SDK=$TRAKTOR_HOME/3rdp/jpeg/jpeg-9e
export LUA_SDK=$TRAKTOR_HOME/3rdp/lua/lua-5.4.6
export LZF_SDK=$TRAKTOR_HOME/3rdp/liblzf/liblzf-3.6
export MIKKTSPACE_SDK=$TRAKTOR_HOME/3rdp/MikkTSpace
export OGG_SDK=$TRAKTOR_HOME/3rdp/ogg
export P4_SDK=$TRAKTOR_HOME/3rdp/p4api-linux/p4api
export PNG_SDK=$TRAKTOR_HOME/3rdp/libpng
export RAPIDJSON_SDK=$TRAKTOR_HOME/3rdp/rapidjson
export RECAST_SDK=$TRAKTOR_HOME/3rdp/recastnavigation
export RENDERDOC_SDK=$TRAKTOR_HOME/3rdp/renderdoc/renderdoc_1.29
export RG_ETC1_SDK=$TRAKTOR_HOME/3rdp/rg-etc1
export STB_SDK=$TRAKTOR_HOME/3rdp/stb
export STEAMWORKS_SDK=$TRAKTOR_HOME/3rdp/steamworks/sdk
export SQLITE_SDK=$TRAKTOR_HOME/3rdp/sqlite/sqlite-amalgamation-3390200
export SQUISH_SDK=$TRAKTOR_HOME/3rdp/squish
export THEORA_SDK=$TRAKTOR_HOME/3rdp/theora
export TINYEXR_SDK=$TRAKTOR_HOME/3rdp/tinyexr
export VORBIS_SDK=$TRAKTOR_HOME/3rdp/vorbis
export XATLAS_SDK=$TRAKTOR_HOME/3rdp/xatlas
export ZLIB_SDK=$TRAKTOR_HOME/3rdp/zlib
export BC6H_ENC_SDK=$TRAKTOR_HOME/3rdp/bc6h_enc
export MINIMP3_SDK=$TRAKTOR_HOME/3rdp/minimp3
export MIMALLOC_SDK=$TRAKTOR_HOME/3rdp/mimalloc

# Vulkan SDK
# - cleanup Extern and add Extern.vulkan which contain all paths,
# and replace in Traktor.Vulkan and Traktor.Vulkan.Editor
# VULKAN_SDK
#	VULKAN_HEADERS_SDK
#	VULKAN_VMA_SDK
#	VULKAN_GLSLANG_SDK
#	VULKAN_SPIRVTOOLS_SDK
#	VULKAN_SPIRVCROSS_SDK
export VULKAN_BASE_SDK=$TRAKTOR_HOME/3rdp/vulkan-linux/latest/x86_64
export VULKAN_SDK=$VULKAN_BASE_SDK
export GLSLANG_SDK=$VULKAN_BASE_SDK/include/glslang
export SPIRVTOOLS_SDK=$VULKAN_BASE_SDK
export SPIRVCROSS_SDK=$VULKAN_BASE_SDK/include/spirv_cross

# Vulkan Memory Allocator
export VMA_SDK=$TRAKTOR_HOME/3rdp/VulkanMemoryAllocator

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
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp/embree-linux
elif [[ $TRAKTOR_PLATFORM == macos ]]; then
	export EMBREE_SDK=$TRAKTOR_HOME/3rdp/embree-macOS
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

# Android SDK
if [[ `uname -s` == Linux* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp/android-sdk-linux
elif [[ `uname -s` == Darwin* ]]; then
	export ANDROID_HOME=$TRAKTOR_HOME/3rdp/android-sdk-macOS
fi
export ANDROID_NDK_ROOT=$ANDROID_HOME/ndk-bundle
