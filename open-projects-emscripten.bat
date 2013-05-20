@echo off

call %~dp0config.bat

start build\emscripten\"Traktor Emscripten.sln"
