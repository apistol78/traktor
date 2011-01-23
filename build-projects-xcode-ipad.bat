@echo off
call %~dp0config.bat
%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode TraktorIPad.xms -d=DebugStatic -r=ReleaseStatic -a -i=ipad
