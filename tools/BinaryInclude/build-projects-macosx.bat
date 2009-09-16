@echo off

call %~dp0..\..\config.bat

mkdir build

%TRAKTOR_HOME%\bin\solutionbuilder -f=make BinaryIncludeMacOSX.xms -d=gnu -p=osx
