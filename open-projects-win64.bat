@echo off

call %~dp0config.bat

start build\win64\"Traktor Win64.sln"
