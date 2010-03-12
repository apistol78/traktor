@echo off

call %~dp0config.bat

start build\ps3\"Traktor Ps3.sln"
start build\ps3-spu\"Traktor Ps3 SPU.sln"
