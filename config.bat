@echo off

set TRAKTOR_HOME=%~dp0

set STEAMWORKS_SDK=%TRAKTOR_HOME%3rdp\steamworks-sdk-128
set FBX_SDK=%TRAKTOR_HOME%3rdp\FbxSdk\2013.1
set BULLET_SDK=%TRAKTOR_HOME%3rdp\bullet3-master
set PHYSX_SDK=%TRAKTOR_HOME%3rdp\PhysX-3.3.1
set ANGLE_SDK=%TRAKTOR_HOME%3rdp\angle-2014-05-09
set P4_X86_SDK=%TRAKTOR_HOME%3rdp\p4api-2013.3.707972.BETA-vs2008_dyn_x86
set P4_X64_SDK=%TRAKTOR_HOME%3rdp\p4api-2013.3.707972.BETA-vs2008_dyn_x64

:: set ANT_HOME=C:\apache-ant-1.9.1
:: set ANDROID_HOME=C:\android-sdk
:: set ANDROID_NDK_ROOT=C:\android-ndk-r10b

:: set NACL_SDK_ROOT=%TRAKTOR_HOME%3rdp\nacl_sdk\pepper_37
:: set CHROME_PATH="C:\Program Files (x86)\Google\Chrome"

set PATH=%PATH%;%STEAMWORKS_SDK%\redistributable_bin
set PATH=%PATH%;%STEAMWORKS_SDK%\redistributable_bin\win64
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\DirectX Redist (8.0)\x86
set PATH=%PATH%;%SCE_PS3_ROOT%\host-win32\Cg\bin
set PATH=%PATH%;%FBX_SDK%\lib\vs2008\x86
set PATH=%PATH%;%PHYSX_SDK%\bin\win32
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Builds\OGLES2\WindowsPC\Lib
set PATH=%PATH%;%TRAKTOR_HOME%3rdp\POWERVR SDK\OGLES2_WINDOWS_PCEMULATION_2.07.27.0484\Utilities\PVRTC\dll
