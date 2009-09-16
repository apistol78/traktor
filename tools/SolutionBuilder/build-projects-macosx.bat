@echo off

call %~dp0..\..\config.bat

mkdir build

%TRAKTOR_HOME%\bin\solutionbuilder -f=make SolutionBuilderMacOSX.xms -d=gnu -p=osx
