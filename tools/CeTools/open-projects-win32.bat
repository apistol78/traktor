@echo off

call %~dp0../../config.bat

start build\win32\"CeTools Win32.sln"
