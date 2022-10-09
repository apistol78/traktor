@echo off

:: Get Traktor home path using some bat magic.
for %%i in ("%~dp0.") do (set TRAKTOR_HOME=%%~dpi)
if %TRAKTOR_HOME:~-1%==\ set TRAKTOR_HOME=%TRAKTOR_HOME:~0,-1%

rem :: Paths to 3rd party dependencies.
rem set STEAMWORKS_SDK=%TRAKTOR_3RDP%\steamworks-sdk-142
rem set DETOURS_SDK=%TRAKTOR_3RDP%\Detours-master-180809
rem set FBX_SDK=%TRAKTOR_3RDP%\FbxSdk\2016.1.2
rem set BULLET_SDK=%TRAKTOR_3RDP%\bullet3-master-210317
rem set PHYSX_SDK=%TRAKTOR_3RDP%\PhysX-3.3.1
rem set GLSLANG=%TRAKTOR_3RDP%\glslang-master-200605
rem set SQLITE_SDK=%TRAKTOR_3RDP%\sqlite-3.8.7.2
rem set SIMPLYGON_SDK=%TRAKTOR_3RDP%\SimplygonSDK
rem set LUA_SDK=%TRAKTOR_3RDP%\lua-5.4.3
rem set OPENVR_SDK=%TRAKTOR_3RDP%\openvr-master-20160822
rem set EXPAT_SDK=%TRAKTOR_3RDP%\libexpat-R_2_2_5\expat
rem set ZLIB_SDK=%TRAKTOR_3RDP%\zlib-1.2.11
rem set EMBREE_SDK=%TRAKTOR_3RDP%\embree-3.5.2.x64.vc14.windows
rem set OIDN_SDK=%TRAKTOR_3RDP%\oidn-1.3.0.x64.vc14.windows
rem set VULKAN_SDK=%TRAKTOR_3RDP%\vulkan\Vulkan-Headers-master-201012
rem set VMA_SDK=%TRAKTOR_3RDP%\VulkanMemoryAllocator-master-210504
rem set SPIRVTOOLS_SDK=%TRAKTOR_3RDP%\SPIRV-Tools-master-201029
rem set SPIRVCROSS_SDK=%TRAKTOR_3RDP%\SPIRV-Cross-master-201009
rem set ASTCENC_SDK=%TRAKTOR_3RDP%\astc-encoder-master-201104
rem set MIKKTSPACE_SDK=%TRAKTOR_3RDP%\MikkTSpace-master-200911
rem set AGG_SDK=%TRAKTOR_3RDP%\agg-2.5

:: Paths to 3rd party dependencies.
set RPI_SYSROOT=%TRAKTOR_HOME%\3rdp\raspberry-sysroot
set MOLTEN_SDK=%TRAKTOR_HOME%\3rdp\vulkan\vulkansdk-macos-1.2.154.0\MoltenVK 

set AGG_SDK=%TRAKTOR_HOME%\3rdp_\agg-2.6
set ASTCENC_SDK=%TRAKTOR_HOME%\3rdp_\astc-encoder
set BULLET_SDK=%TRAKTOR_HOME%\3rdp_\bullet3
set EMBREE_SDK=%TRAKTOR_HOME%\3rdp_\embree-windows\embree-3.13.4.x64.vc14.windows
set EXPAT_SDK=%TRAKTOR_HOME%\3rdp_\libexpat\expat
set FBX_SDK=%TRAKTOR_HOME%\3rdp_\fbx\2016.1.2
set FLAC_SDK=%TRAKTOR_HOME%\3rdp_\flac
set FREETYPE_SDK=%TRAKTOR_HOME%\3rdp_\freetype
set JPEG_SDK=%TRAKTOR_HOME%\3rdp_\jpeg\jpeg-9e
set LUA_SDK=%TRAKTOR_HOME%\3rdp_\lua\lua-5.4.4
set LZF_SDK=%TRAKTOR_HOME%\3rdp_\liblzf\liblzf-3.6
set LZO_SDK=%TRAKTOR_HOME%\3rdp_\lzo\lzo-2.10
set MIKKTSPACE_SDK=%TRAKTOR_HOME%\3rdp_\MikkTSpace
set OGG_SDK=%TRAKTOR_HOME%\3rdp_\ogg
set OIDN_SDK=%TRAKTOR_HOME%\3rdp_\oidn-windows\oidn-1.4.3.x64.vc14.windows
set P4_SDK=%TRAKTOR_HOME%\3rdp_\p4api-linux\p4api-2022.1.2305383
set PNG_SDK=%TRAKTOR_HOME%\3rdp_\libpng
set RAPIDJSON_SDK=%TRAKTOR_HOME%\3rdp_\rapidjson
set RECAST_SDK=%TRAKTOR_HOME%\3rdp_\recastnavigation
set RG_ETC1_SDK=%TRAKTOR_HOME%\3rdp_\rg-etc1
set STB_SDK=%TRAKTOR_HOME%\3rdp_\stb
set STEAMWORKS_SDK=%TRAKTOR_HOME%\3rdp_\steamworks\sdk
set SQLITE_SDK=%TRAKTOR_HOME%\3rdp_\sqlite\sqlite-amalgamation-3390200
set SQUISH_SDK=%TRAKTOR_HOME%\3rdp_\squish
set THEORA_SDK=%TRAKTOR_HOME%\3rdp_\theora
set TINYEXR_SDK=%TRAKTOR_HOME%\3rdp_\tinyexr
set VORBIS_SDK=%TRAKTOR_HOME%\3rdp_\vorbis
set XATLAS_SDK=%TRAKTOR_HOME%\3rdp_\xatlas
set ZLIB_SDK=%TRAKTOR_HOME%\3rdp_\zlib

:: Vulkan SDK
set VULKAN_SDK=%TRAKTOR_HOME%\3rdp_\vulkan-windows\VulkanSDK-1.3.216.0
set VMA_SDK=%TRAKTOR_HOME%\3rdp_\vulkan-windows\VulkanSDK-1.3.216.0\include\vma

:: Export name of solution builder binary.
set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

:: Export path to Ninja binary.
set NINJA=%TRAKTOR_HOME%\3rdp_\ninja-windows\ninja

:: Export path to FBuild executable.
set FBUILD=%TRAKTOR_HOME%\3rdp\FASTBuild-Windows-x64-v1.05\FBuild.exe
