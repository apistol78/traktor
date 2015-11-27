@echo off

call %~dp0config.bat

:: Set aggregate output path if not already defined.
if "%TRAKTOR_AGGREGATE_OUTPUT_PATH%" == "" (
	set TRAKTOR_AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win64
)

%TRAKTOR_HOME%\bin\win32\solutionbuilder -f=msvc -p=$(TRAKTOR_HOME)\bin\msvc-2012-win64.xml %TRAKTOR_HOME%\TraktorWin64.xms
