@echo off

call %~dp0../../config.bat

start build\win32\"SourceVerify Win32.sln"
