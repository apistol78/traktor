@echo off

call %~dp0config.bat

start %TRAKTOR_HOME%\build\ps3\"Traktor Ps3.sln"
start %TRAKTOR_HOME%\build\ps3-spu\"Traktor Ps3 SPU.sln"
