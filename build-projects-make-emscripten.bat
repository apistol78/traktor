@echo off

call %~dp0config.bat

mkdir %TRAKTOR_HOME%\build

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=make -d=nmake -p=linux -c=$(TRAKTOR_HOME)\bin\make-config-emscripten.inc %TRAKTOR_HOME%\TraktorEmscripten.xms
