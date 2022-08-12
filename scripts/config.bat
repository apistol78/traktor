@echo off

:: Get Traktor home path using some bat magic.
for %%i in ("%~dp0.") do (set TRAKTOR_HOME=%%~dpi)
if %TRAKTOR_HOME:~-1%==\ set TRAKTOR_HOME=%TRAKTOR_HOME:~0,-1%

:: Path to 3rd party packages.
export TRAKTOR_3RDP=%TRAKTOR_HOME%\3rdp

:: Paths to 3rd party dependencies.
set STEAMWORKS_SDK=%TRAKTOR_3RDP%\steamworks-sdk-142
set DETOURS_SDK=%TRAKTOR_3RDP%\Detours-master-180809
set FBX_SDK=%TRAKTOR_3RDP%\FbxSdk\2016.1.2
set BULLET_SDK=%TRAKTOR_3RDP%\bullet3-master-210317
set PHYSX_SDK=%TRAKTOR_3RDP%\PhysX-3.3.1
set GLSLANG=%TRAKTOR_3RDP%\glslang-master-200605
set SQLITE_SDK=%TRAKTOR_3RDP%\sqlite-3.8.7.2
set SIMPLYGON_SDK=%TRAKTOR_3RDP%\SimplygonSDK
set LUA_SDK=%TRAKTOR_3RDP%\lua-5.4.3
set OPENVR_SDK=%TRAKTOR_3RDP%\openvr-master-20160822
set EXPAT_SDK=%TRAKTOR_3RDP%\libexpat-R_2_2_5\expat
set ZLIB_SDK=%TRAKTOR_3RDP%\zlib-1.2.11
set SCE_PS3_ROOT=%TRAKTOR_3RDP%\ps3\usr\local\cell
set SN_COMMON_PATH=%TRAKTOR_3RDP%\ps3\SN Systems\Common
set SN_PS3_PATH=%TRAKTOR_3RDP%\ps3\SN Systems\Ps3
set EMBREE_SDK=%TRAKTOR_3RDP%\embree-3.5.2.x64.vc14.windows
set OIDN_SDK=%TRAKTOR_3RDP%\oidn-1.3.0.x64.vc14.windows
set VULKAN_SDK=%TRAKTOR_3RDP%\vulkan\Vulkan-Headers-master-201012
set VMA_SDK=%TRAKTOR_3RDP%\VulkanMemoryAllocator-master-210504
set SPIRVTOOLS_SDK=%TRAKTOR_3RDP%\SPIRV-Tools-master-201029
set SPIRVCROSS_SDK=%TRAKTOR_3RDP%\SPIRV-Cross-master-201009
set ASTCENC_SDK=%TRAKTOR_3RDP%\astc-encoder-master-201104
set MIKKTSPACE_SDK=%TRAKTOR_3RDP%\MikkTSpace-master-200911
set AGG_SDK=%TRAKTOR_3RDP%\agg-2.5

:: Export name of solution builder binary.
set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

:: Export path to Ninja binary.
set NINJA=%TRAKTOR_3RDP%\ninja\win\ninja

:: Export path to FBuild executable.
set FBUILD=%TRAKTOR_3RDP%\FASTBuild-Windows-x64-v1.05\FBuild.exe
