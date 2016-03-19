@echo off

set TRAKTOR_HOME=%~dp0

set STEAMWORKS_SDK=%TRAKTOR_HOME%3rdp\steamworks-sdk-135
set FBX_SDK=%TRAKTOR_HOME%3rdp\FbxSdk\2013.1
set BULLET_SDK=%TRAKTOR_HOME%3rdp\bullet3-master
set PHYSX_SDK=%TRAKTOR_HOME%3rdp\PhysX-3.3.1
set ANGLE_SDK=%TRAKTOR_HOME%3rdp\angle-2014-05-09
set GLSL_OPTIMIZER=%TRAKTOR_HOME%3rdp\glsl-optimizer-master-20141020
set P4_X86_SDK=%TRAKTOR_HOME%3rdp\p4api-2013.3.707972.BETA-vs2008_dyn_x86
set P4_X64_SDK=%TRAKTOR_HOME%3rdp\p4api-2013.3.707972.BETA-vs2008_dyn_x64
set SQLITE_SDK=%TRAKTOR_HOME%3rdp\sqlite-3.8.7.2
set WXWIDGETS_SDK=%TRAKTOR_HOME%3rdp\wxWidgets-3.0.2
set SIMPLYGON_SDK=%TRAKTOR_HOME%3rdp\SimplygonSDK
set LUA_SDK=%TRAKTOR_HOME%3rdp\lua-5.3.2
set GLEW_SDK=%TRAKTOR_HOME%3rdp\glew-1.13.0
set TOBII_SDK=%TRAKTOR_HOME%3rdp\tobii-sdk-1.7.480

:: PNaCl SDK; need to be manually installed in 3rdp folder.
set NACL_SDK_ROOT=%TRAKTOR_HOME%3rdp\nacl_sdk\pepper_47
set CHROME_PATH=