@echo off

:: Setup VC environment variables.
call "%VS140COMNTOOLS%..\..\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Remove old log.
del /F /Q %~dp0autobuild-android.log

:: Set aggregate output path if not already defined.
if "%AGGREGATE_OUTPUT_PATH%" == "" (
	set AGGREGATE_OUTPUT_PATH=%TRAKTOR_HOME%\bin\latest\android
)

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2015-android.bat
devenv "build\android\Traktor Android.sln" /Build DebugStatic /Out %~dp0autobuild-android.log
devenv "build\android\Traktor Android.sln" /Build ReleaseStatic /Out %~dp0autobuild-android.log
popd
