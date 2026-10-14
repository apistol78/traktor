@echo off

call %~dp0config.bat

set SOLUTIONBUILDER=%TRAKTOR_HOME%\bin\win64\releasestatic\Traktor.SolutionBuilder.App

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2026-win64.xml ^
	%TRAKTOR_HOME%\resources\build\ExternWin64.xms
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%

%SOLUTIONBUILDER% ^
	-f=msvc ^
	-i ^
	-p=$(TRAKTOR_HOME)\resources\build\configurations\msvc-2026-win64.xml ^
	%TRAKTOR_HOME%\resources\build\TraktorWin64.xms
if %ERRORLEVEL% neq 0 exit /b %ERRORLEVEL%
