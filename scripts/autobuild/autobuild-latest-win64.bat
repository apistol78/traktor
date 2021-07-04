@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-x64.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win64"
%FBUILD% -config "Extern Win64.bff" -noprogress -summary -cache %CONFIG%
%FBUILD% -config "Traktor Win64.bff" -noprogress -summary -cache %CONFIG%
popd
