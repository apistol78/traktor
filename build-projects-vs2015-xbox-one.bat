@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%SOLUTIONBUILDER% -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2015-xbox-one.xml %TRAKTOR_HOME%\TraktorXboxOne.xms
