@echo off

call "<!-- output:print(os:getEnvironment("TRAKTOR_HOME")) --!>\scripts\config.bat"

start build\win64\"<!-- output:print(os:getEnvironment("WIZARD_NAME")) --!> Win64.sln"
