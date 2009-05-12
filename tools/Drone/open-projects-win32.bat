@echo off

call %~dp0../../config.bat

start build\win32\"Drone Win32.sln"
