@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\emscripten"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Emscripten.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Emscripten.mak" %CONFIG%
popd
