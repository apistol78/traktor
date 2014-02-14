@echo off

call %~dp0config.bat

start build\linux\"Traktor Linux.sln"
