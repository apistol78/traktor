@echo off

:: Add Emscripten SDK to environment.
pushd "C:\Program Files (x86)\Emscripten"
call emsdk_add_path.bat
popd

node $1
