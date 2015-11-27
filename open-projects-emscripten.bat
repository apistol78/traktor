@echo off

:: Setup default environment.
call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\emscripten

:: Add Emscripten SDK to environment.
if exist "C:\Program Files\Emscripten\emsdk_env.bat" (
	pushd "C:\Program Files\Emscripten"
	call emsdk_env.bat
	popd
)
if exist "C:\Program Files (x86)\Emscripten\emsdk_env.bat" (
	pushd "C:\Program Files (x86)\Emscripten"
	call emsdk_env.bat
	popd
)

start build\emscripten\"Traktor Emscripten.sln"
