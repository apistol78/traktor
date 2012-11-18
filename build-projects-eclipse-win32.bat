@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=eclipse -p=$(TRAKTOR_HOME)/bin/eclipse-msvc-project.sb -c=$(TRAKTOR_HOME)/bin/eclipse-msvc-cproject.sb %TRAKTOR_HOME%\TraktorWin32.xms
