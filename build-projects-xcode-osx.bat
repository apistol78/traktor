@echo off
call %~dp0config.bat
%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared
