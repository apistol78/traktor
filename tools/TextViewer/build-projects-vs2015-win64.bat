@echo off

call %~dp0../../../scripts/config.bat

mkdir %~dp0build

%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2015-win64.xml TextViewerWin64.xms
