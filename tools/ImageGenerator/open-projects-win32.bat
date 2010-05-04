@echo off

call %~dp0../../config.bat

start build\win32\"ImageGenerator Win32.sln"
