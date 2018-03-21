@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Remove old log.
del /F /Q %~dp0autobuild-ps3.log

:: Build Traktor.
pushd "%TRAKTOR_HOME%\build\ps3"
devenv "Traktor Ps3.sln" /Build DebugStatic /Out %~dp0autobuild-ps3.log
devenv "Traktor Ps3.sln" /Build ReleaseStatic /Out %~dp0autobuild-ps3.log
popd
pushd "%TRAKTOR_HOME%\build\ps3-spu"
devenv "Traktor Ps3 SPU.sln" /Build DebugStatic /Out %~dp0autobuild-ps3.log
devenv "Traktor Ps3 SPU.sln" /Build ReleaseStatic /Out %~dp0autobuild-ps3.log
popd
