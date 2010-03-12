@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-ps3.xml %TRAKTOR_HOME%\TraktorPs3.xms
%TRAKTOR_HOME%\bin\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2008-ps3-spu.xml %TRAKTOR_HOME%\TraktorPs3-Spu.xms
