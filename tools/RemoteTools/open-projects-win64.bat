@echo off

call %~dp0../../config.bat

start build\win64\"RemoteTools Win64.sln"
