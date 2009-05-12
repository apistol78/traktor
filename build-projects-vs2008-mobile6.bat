@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-mobile6.xml %TRAKTOR_HOME%\TraktorMobile6.xms
