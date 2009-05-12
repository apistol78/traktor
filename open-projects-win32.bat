@echo off

call %~dp0config.bat

start build\win32\"Traktor Win32.sln"
