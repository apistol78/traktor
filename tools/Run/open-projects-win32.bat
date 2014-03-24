@echo off

call %~dp0../../config.bat

start build\win32\"Run Win32.sln"
