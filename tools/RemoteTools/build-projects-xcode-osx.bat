@echo off
call %~dp0../../config.bat
mkdir %~dp0build
%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode RemoteToolsMacOSX.xms -d=DebugShared -r=ReleaseStatic -a

