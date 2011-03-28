@echo off
call %~dp0config.bat
if "%1"=="static" (
	echo Static solution
	%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode TraktorMacOSX.xms -d=DebugStatic -r=ReleaseStatic -a
) else (
	echo Shared solution
	%TRAKTOR_HOME%\bin\solutionbuilder -f=xcode TraktorMacOSX.xms -d=DebugShared -r=ReleaseShared -a
)

