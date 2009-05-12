@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms
