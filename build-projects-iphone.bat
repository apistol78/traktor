@echo off
call %~dp0config.bat
%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode TraktorIPhone.xms -d=DebugStatic -r=ReleaseStatic -i
