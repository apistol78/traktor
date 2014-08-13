@echo off

rem Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

rem Setup our build environment.
call %~dp0../config.bat

rem Remove old log.
del /F /Q %~dp0autobuild-win32.log

rem Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2012-win32.bat
devenv "build\win32\Traktor Win32.sln" /Build DebugShared /Out %~dp0autobuild-win32.log
devenv "build\win32\Traktor Win32.sln" /Build ReleaseShared /Out %~dp0autobuild-win32.log
popd

call copy-latest-win32.bat
pause