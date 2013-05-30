@echo off

call %~dp0../../config.bat

start build\win32\"NetworkDebugger Win32.sln"
