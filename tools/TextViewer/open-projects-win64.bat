@echo off

call %~dp0../../scripts/config.bat

start build\win64\"TextViewer.sln"
