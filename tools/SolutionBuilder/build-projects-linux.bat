@echo off

call %~dp0..\..\config.bat

mkdir build

%TRAKTOR_HOME%\bin\solutionbuilder -f=make SolutionBuilderLinux.xms -d=gnu -p=linux
