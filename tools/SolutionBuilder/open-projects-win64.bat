@echo off

call %~dp0config.bat

start build\win64\SolutionBuilder.sln
