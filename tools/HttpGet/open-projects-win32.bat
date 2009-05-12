@echo off

call %~dp0../../config.bat

start build\win32\"HttpGet Win32.sln"
