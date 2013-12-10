@echo off

:: Setup default environment.
call %~dp0config.bat

:: Add Emscripten SDK to environment.
pushd "C:\Program Files (x86)\Emscripten"
call emsdk_add_path.bat
popd

start build\emscripten\"Traktor Emscripten.sln"
