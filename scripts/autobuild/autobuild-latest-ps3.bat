@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-x86.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\ps3"
%TRAKTOR_HOME%\3rdp\jom\jom /L /F "Extern Ps3.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Ps3.mak" %CONFIG%
popd
pushd "%TRAKTOR_HOME%\build\ps3-spu"
%TRAKTOR_HOME%\3rdp\jom\jom /L /F "Traktor Ps3 SPU.mak" %CONFIG%
popd
