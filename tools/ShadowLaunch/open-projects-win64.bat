@echo off

call %~dp0../../config.bat

start build\win64\"ShadowLaunch Win64.sln"
