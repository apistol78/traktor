@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Remove old log.
del /F /Q %~dp0autobuild-ps3.log

:: Rebuild entire solution.
pushd %TRAKTOR_HOME%
call scripts\build-projects-vs2012-ps3.bat
devenv "build\ps3\Traktor Ps3.sln" /Build DebugStatic /Out %~dp0autobuild-ps3.log
devenv "build\ps3\Traktor Ps3.sln" /Build ReleaseStatic /Out %~dp0autobuild-ps3.log
devenv "build\ps3-spu\Traktor Ps3 SPU.sln" /Build DebugStatic /Out %~dp0autobuild-ps3.log
devenv "build\ps3-spu\Traktor Ps3 SPU.sln" /Build ReleaseStatic /Out %~dp0autobuild-ps3.log
popd
