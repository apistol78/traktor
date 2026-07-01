@echo off

call "<!-- output:print(os:getEnvironment("TRAKTOR_HOME")) --!>\scripts\config.bat"

%SOLUTIONBUILDER% ^
	-f=msvc ^
	"-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2022-win64.xml" ^
	"<!-- output:print(os:getEnvironment("WIZARD_NAME")) --!>Win64.xms"
