@echo off

call %~dp0..\..\scripts\config.bat

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2022-win64.xml ^
	%~dp0WebberWin64.xms
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
