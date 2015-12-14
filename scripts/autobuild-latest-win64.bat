@echo off

:: Setup VC environment variables.
call "%VS110COMNTOOLS%..\..\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Remove old log.
del /F /Q %~dp0autobuild-win64.log

:: Set aggregate output path if not already defined.
if "%AGGREGATE_OUTPUT_PATH%" == "" (
	set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\win64
)

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2012-win64.bat
devenv "build\win64\Traktor Win64.sln" /Build DebugShared /Out %~dp0autobuild-win64.log
devenv "build\win64\Traktor Win64.sln" /Build ReleaseShared /Out %~dp0autobuild-win64.log
devenv "build\win64\Traktor Win64.sln" /Build DebugStatic /Out %~dp0autobuild-win64.log
devenv "build\win64\Traktor Win64.sln" /Build ReleaseStatic /Out %~dp0autobuild-win64.log
popd

:: call copy-latest-win64.bat
pause