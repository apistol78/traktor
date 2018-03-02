@echo off

call %~dp0config.bat

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc "-p=$(TRAKTOR_HOME)\bin\msvc-2012-win64.xml" <!-- output:print(os:getEnvironment("WIZARD_NAME")) --!>Win64.xms
