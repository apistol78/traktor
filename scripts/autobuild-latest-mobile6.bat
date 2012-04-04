@echo off

rem Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat"

rem Setup our build environment.
call %~dp0../config.bat

rem Remove old log.
del /F /Q %~dp0autobuild-mobile6.log

rem Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2008-mobile6.bat
devenv "build\mobile6\Traktor Mobile6.sln" /Build DebugShared /Out %~dp0autobuild-mobile6.log
devenv "build\mobile6\Traktor Mobile6.sln" /Build ReleaseShared /Out %~dp0autobuild-mobile6.log
popd

call copy-latest-mobile6.bat
pause