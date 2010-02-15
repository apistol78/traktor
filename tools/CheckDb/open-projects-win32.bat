@echo off

call %~dp0../../config.bat

start build\win32\"CheckDb Win32.sln"
