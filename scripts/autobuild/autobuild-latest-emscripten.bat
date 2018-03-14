@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-emscripten.bat"
cd "build\emscripten"
nmake -f "Extern Emscripten.mak"
nmake -f "Traktor Emscripten.mak"
popd
