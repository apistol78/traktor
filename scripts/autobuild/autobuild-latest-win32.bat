@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-x86.bat"

:: FBuild executable.
set FBUILD=%TRAKTOR_HOME%\3rdp\FASTBuild-Windows-x64-v0.95\FBuild.exe

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win32"
%FBUILD% -config "Extern Win32.bff" -noprogress -summary -cache %CONFIG%
%FBUILD% -config "Traktor Win32.bff" -noprogress -summary -cache %CONFIG%
popd

