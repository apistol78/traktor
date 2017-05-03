@echo off

call %~dp0../../config.bat

mkdir %~dp0build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-win64.xml FlashRemoteDebuggerWin64.xms
