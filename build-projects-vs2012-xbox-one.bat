@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\win64\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-xbox-one.xml %TRAKTOR_HOME%\TraktorXboxOne.xms
