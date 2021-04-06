@echo off

:: Get Traktor home path using some bat magic.
for %%i in ("%~dp0.") do (set TRAKTOR_HOME=%%~dpi)
if %TRAKTOR_HOME:~-1%==\ set TRAKTOR_HOME=%TRAKTOR_HOME:~0,-1%

:: Paths to 3rd party dependencies.
set STEAMWORKS_SDK=%TRAKTOR_HOME%\3rdp\steamworks-sdk-142
set DETOURS_SDK=%TRAKTOR_HOME%\3rdp\Detours-master-180809
set FBX_SDK=%TRAKTOR_HOME%\3rdp\FbxSdk\2016.1.2
set BULLET_SDK=%TRAKTOR_HOME%\3rdp\bullet3-master-210317
set PHYSX_SDK=%TRAKTOR_HOME%\3rdp\PhysX-3.3.1
set ANGLE_SDK=%TRAKTOR_HOME%\3rdp\angle-master-160921
set GLSL_OPTIMIZER=%TRAKTOR_HOME%\3rdp\glsl-optimizer-master-20160321
set GLSLANG=%TRAKTOR_HOME%\3rdp\glslang-master-200605
set SQLITE_SDK=%TRAKTOR_HOME%\3rdp\sqlite-3.8.7.2
set SIMPLYGON_SDK=%TRAKTOR_HOME%\3rdp\SimplygonSDK
set LUA_SDK=%TRAKTOR_HOME%\3rdp\lua-5.3.5
set GLEW_SDK=%TRAKTOR_HOME%\3rdp\glew-1.13.0
set TOBII_SDK=%TRAKTOR_HOME%\3rdp\tobii-sdk-1.7.480
set OPENVR_SDK=%TRAKTOR_HOME%\3rdp\openvr-master-20160822
set EXPAT_SDK=%TRAKTOR_HOME%\3rdp\libexpat-R_2_2_5\expat
set ZLIB_SDK=%TRAKTOR_HOME%\3rdp\zlib-1.2.11
set SCE_PS3_ROOT=%TRAKTOR_HOME%\3rdp\ps3\usr\local\cell
set SN_COMMON_PATH=%TRAKTOR_HOME%\3rdp\ps3\SN Systems\Common
set SN_PS3_PATH=%TRAKTOR_HOME%\3rdp\ps3\SN Systems\Ps3
set EMBREE_SDK=%TRAKTOR_HOME%\3rdp\embree-3.5.2.x64.vc14.windows
set OIDN_SDK=%TRAKTOR_HOME%\3rdp\oidn-1.3.0.x64.vc14.windows
set VULKAN_SDK=%TRAKTOR_HOME%\3rdp\vulkan\Vulkan-Headers-master-201012
set VMA_SDK=%TRAKTOR_HOME%\3rdp\VulkanMemoryAllocator-master
set SPIRVTOOLS_SDK=%TRAKTOR_HOME%\3rdp\SPIRV-Tools-master-201029
set SPIRVCROSS_SDK=%TRAKTOR_HOME%\3rdp\SPIRV-Cross-master-201009
set ASTCENC_SDK=%TRAKTOR_HOME%\3rdp\astc-encoder-master-201104

:: Export name of solution builder binary.
set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

:: Export name of shadow launch binary.
set SHADOWLAUNCH=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.ShadowLaunch.App

:: Export path to Ninja binary.
set NINJA=%TRAKTOR_HOME%\3rdp\ninja\win\ninja
