@echo off

call %~dp0config.bat

rem Add Emscripten SDK to path.
pushd "C:\Program Files (x86)\Emscripten"
call emsdk_add_path.bat
popd

start build\emscripten\"Traktor Emscripten.sln"
