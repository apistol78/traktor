@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-android.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\android"
%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Extern Android.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Traktor Android.mak" %CONFIG%
popd
