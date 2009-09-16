@echo off


call %~dp0config.bat


mkdir %TRAKTOR_HOME%\build


%TRAKTOR_HOME%\bin\solutionbuilder -f=make TraktorMacOSX.xms -d=gnu -p=osx