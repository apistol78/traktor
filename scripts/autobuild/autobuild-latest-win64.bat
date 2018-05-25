@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-x64.bat"

:: FBuild executable.
set FBUILD=%TRAKTOR_HOME%\3rdp\FASTBuild-Windows-x64-v0.95\FBuild.exe

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win64"
%FBUILD% -config "Extern Win64.bff" -noprogress -summary -cache %CONFIG%
%FBUILD% -config "Traktor Win64.bff" -noprogress -summary -cache %CONFIG%
popd
