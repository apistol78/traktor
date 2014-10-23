@echo off

:: Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Remove old log.
del /F /Q %~dp0autobuild-android.log

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2012-android.bat
devenv "build\android\Traktor Android.sln" /Build DebugStatic /Out %~dp0autobuild-android.log
devenv "build\android\Traktor Android.sln" /Build ReleaseStatic /Out %~dp0autobuild-android.log
popd

call copy-latest-android.bat
pause
