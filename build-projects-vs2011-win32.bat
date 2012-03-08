@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2011-win32.xml %TRAKTOR_HOME%\TraktorWin32.xms
