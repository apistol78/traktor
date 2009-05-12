@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -p=%TRAKTOR_HOME%\bin\ps3.xml %TRAKTOR_HOME%\TraktorPs3.xms
