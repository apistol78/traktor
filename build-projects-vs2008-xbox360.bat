@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-xbox360.xml %TRAKTOR_HOME%\TraktorXbox360.xms
