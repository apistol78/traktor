@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win32"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Win32.mak" %CONFIG%
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Win32.mak" %CONFIG%
popd

