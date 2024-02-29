@echo off

:: Get Traktor home path using some bat magic.
for %%i in ("%~dp0.") do (set TRAKTOR_HOME=%%~dpi)
if %TRAKTOR_HOME:~-1%==\ set TRAKTOR_HOME=%TRAKTOR_HOME:~0,-1%

:: Paths to 3rd party dependencies.
set RPI_SYSROOT=%TRAKTOR_HOME%\3rdp-restricted\raspberry-sysroot
set MOLTEN_SDK=%TRAKTOR_HOME%\3rdp-restricted\vulkan\vulkansdk-macos-1.2.154.0\MoltenVK 

set AGG_SDK=%TRAKTOR_HOME%\3rdp\agg-2.6
set ASTCENC_SDK=%TRAKTOR_HOME%\3rdp\astc-encoder
set BULLET_SDK=%TRAKTOR_HOME%\3rdp\bullet3
set EMBREE_SDK=%TRAKTOR_HOME%\3rdp\embree-windows
set EXPAT_SDK=%TRAKTOR_HOME%\3rdp\libexpat\expat
set FBX_SDK=%TRAKTOR_HOME%\3rdp\fbx\2020.0.1
set FLAC_SDK=%TRAKTOR_HOME%\3rdp\flac
set FREETYPE_SDK=%TRAKTOR_HOME%\3rdp\freetype
set JPEG_SDK=%TRAKTOR_HOME%\3rdp\jpeg\jpeg-9e
set LUA_SDK=%TRAKTOR_HOME%\3rdp\lua\lua-5.4.6
set LZF_SDK=%TRAKTOR_HOME%\3rdp\liblzf\liblzf-3.6
set MIKKTSPACE_SDK=%TRAKTOR_HOME%\3rdp\MikkTSpace
set OGG_SDK=%TRAKTOR_HOME%\3rdp\ogg
::set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-linux\p4api-2022.1.2305383
set PNG_SDK=%TRAKTOR_HOME%\3rdp\libpng
set RAPIDJSON_SDK=%TRAKTOR_HOME%\3rdp\rapidjson
set RECAST_SDK=%TRAKTOR_HOME%\3rdp\recastnavigation
set RENDERDOC_SDK=%TRAKTOR_HOME%\3rdp\renderdoc\RenderDoc_1.26_64
set RG_ETC1_SDK=%TRAKTOR_HOME%\3rdp\rg-etc1
set STB_SDK=%TRAKTOR_HOME%\3rdp\stb
set STEAMWORKS_SDK=%TRAKTOR_HOME%\3rdp\steamworks\sdk
set SQLITE_SDK=%TRAKTOR_HOME%\3rdp\sqlite\sqlite-amalgamation-3390200
set SQUISH_SDK=%TRAKTOR_HOME%\3rdp\squish
set THEORA_SDK=%TRAKTOR_HOME%\3rdp\theora
set TINYEXR_SDK=%TRAKTOR_HOME%\3rdp\tinyexr
set VORBIS_SDK=%TRAKTOR_HOME%\3rdp\vorbis
set XATLAS_SDK=%TRAKTOR_HOME%\3rdp\xatlas
set ZLIB_SDK=%TRAKTOR_HOME%\3rdp\zlib
set DOXYGEN_SDK=%TRAKTOR_HOME%\3rdp\doxygen
set BC6H_ENC_SDK=%TRAKTOR_HOME%\3rdp\bc6h_enc
set MINIMP3_SDK=%TRAKTOR_HOME%\3rdp\minimp3

:: Vulkan SDK
set VULKAN_SDK=%TRAKTOR_HOME%\3rdp\vulkan-windows\vulkan-sdk

:: Vulkan Memory Allocator
set VMA_SDK=%TRAKTOR_HOME%\3rdp\VulkanMemoryAllocator

:: Export name of solution builder binary.
set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

:: Android SDK
set ANDROID_HOME=%TRAKTOR_HOME:\=/%/3rdp/android-sdk-windows
set ANDROID_NDK_ROOT=%ANDROID_HOME:\=/%/ndk-bundle
