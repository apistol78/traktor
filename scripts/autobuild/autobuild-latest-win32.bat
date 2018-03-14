@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-win32.bat"
cd "build\win32"
%TRAKTOR_HOME%\3rdp\jom\jom -j4 -f "Extern Win32.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -j4 -f "Traktor Win32.mak"
popd

