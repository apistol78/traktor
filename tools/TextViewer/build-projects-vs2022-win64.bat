@echo off

call %~dp0../../scripts/config.bat

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

mkdir %~dp0build

%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2022-win64.xml TextViewerWin64.xms
