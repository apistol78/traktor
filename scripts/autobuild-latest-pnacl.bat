@echo off

:: Setup VC environment variables.
call "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"

:: Setup our build environment.
call %~dp0../config.bat

:: Remove old log.
del /F /Q %~dp0autobuild-pnacl.log

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call build-projects-vs2012-pnacl.bat
devenv "build\pnacl\Traktor PNaCl.sln" /Build DebugStatic /Out %~dp0autobuild-pnacl.log
devenv "build\pnacl\Traktor PNaCl.sln" /Build ReleaseStatic /Out %~dp0autobuild-pnacl.log
popd
