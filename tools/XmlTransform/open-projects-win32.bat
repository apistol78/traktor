@echo off

call %~dp0../../config.bat

start build\win32\"XmlTransform Win32.sln"
