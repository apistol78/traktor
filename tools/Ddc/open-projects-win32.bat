@echo off

call %~dp0../../config.bat

start build\win32\"Ddc Win32.sln"
