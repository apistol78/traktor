@echo off

call %~dp0config.bat

start build\win64\"<!-- output:print(os:getEnvironment("WIZARD_NAME")) --!> Win64.sln"
