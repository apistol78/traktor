@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-rpi.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\rpi"
%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Extern Raspberry Pi.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom-windows\jom /L /F "Traktor Raspberry Pi.mak" %CONFIG%
popd
