@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH_PPU=%TRAKTOR_HOME%\bin\latest\ps3
set AGGREGATE_OUTPUT_PATH_SPU=%TRAKTOR_HOME%\bin\latest\ps3-spu

start build\ps3\"Traktor Ps3.sln"
start build\ps3-spu\"Traktor Ps3 SPU.sln"
