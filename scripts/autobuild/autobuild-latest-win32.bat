@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win32"
%TRAKTOR_HOME%\3rdp\jom\jom /J 1 /L /F "Extern Win32.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom\jom /J 1 /L /F "Traktor Win32.mak" %CONFIG%
popd

