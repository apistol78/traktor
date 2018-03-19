@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-emscripten.bat"
cd "build\emscripten"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Emscripten.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Emscripten.mak"
popd
