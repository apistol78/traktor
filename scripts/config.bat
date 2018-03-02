@echo off

:: Get Traktor home path using some bat magic.
for %%i in ("%~dp0.") do (set TRAKTOR_HOME=%%~dpi)
if %TRAKTOR_HOME:~-1%==\ set TRAKTOR_HOME=%TRAKTOR_HOME:~0,-1%

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

:: Paths to 3rd party dependencies.
set STEAMWORKS_SDK=%TRAKTOR_HOME%\3rdp\steamworks-sdk-137
set FBX_SDK=%TRAKTOR_HOME%\3rdp\FbxSdk\2016.1.2
set BULLET_SDK=%TRAKTOR_HOME%\3rdp\bullet3-master-170526
set PHYSX_SDK=%TRAKTOR_HOME%\3rdp\PhysX-3.3.1
set ANGLE_SDK=%TRAKTOR_HOME%\3rdp\angle-master-160921
set GLSL_OPTIMIZER=%TRAKTOR_HOME%\3rdp\glsl-optimizer-master-20160321
set SQLITE_SDK=%TRAKTOR_HOME%\3rdp\sqlite-3.8.7.2
set WXWIDGETS_SDK=%TRAKTOR_HOME%\3rdp\wxWidgets-3.0.2
set SIMPLYGON_SDK=%TRAKTOR_HOME%\3rdp\SimplygonSDK
set LUA_SDK=%TRAKTOR_HOME%\3rdp\lua-5.3.2
set GLEW_SDK=%TRAKTOR_HOME%\3rdp\glew-1.13.0
set TOBII_SDK=%TRAKTOR_HOME%\3rdp\tobii-sdk-1.7.480
set OPENVR_SDK=%TRAKTOR_HOME%\3rdp\openvr-master-20160822
set NACL_SDK_ROOT=%TRAKTOR_HOME\%3rdp\nacl_sdk\pepper_47
set CHROME_PATH=