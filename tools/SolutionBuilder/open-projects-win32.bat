@echo off

call %~dp0config.bat

start build\win32\SolutionBuilder.sln
