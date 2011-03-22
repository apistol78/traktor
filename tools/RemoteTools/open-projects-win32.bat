@echo off

call %~dp0../../config.bat

start build\win32\"RemoteTools Win32.sln"
