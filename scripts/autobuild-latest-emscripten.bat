@echo off

:: Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-make-emscripten.bat
cd build\emscripten
nmake all
popd
