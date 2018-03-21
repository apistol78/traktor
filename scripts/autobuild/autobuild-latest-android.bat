@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\android"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Android.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Android.mak" %CONFIG%
popd
