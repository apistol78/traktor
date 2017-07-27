@echo off

call %~dp0config.bat

mkdir %~dp0build

%TRAKTOR_HOME%\bin\win64\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2017-win64.xml SolutionBuilderWin64.xms
