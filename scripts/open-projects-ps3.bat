@echo off

call %~dp0config.bat
set AGGREGATE_OUTPUT_PATH_PPU=%TRAKTOR_HOME%\bin\latest\ps3\ppu
set AGGREGATE_OUTPUT_PATH_SPU=%TRAKTOR_HOME%\bin\latest\ps3\spu

start %TRAKTOR_HOME%\build\ps3\"Traktor Ps3.sln"
start %TRAKTOR_HOME%\build\ps3-spu\"Traktor Ps3 SPU.sln"
