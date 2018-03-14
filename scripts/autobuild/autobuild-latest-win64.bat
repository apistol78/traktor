@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x64.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-win64.bat"
cd "build\win64"
%TRAKTOR_HOME%\3rdp\jom\jom -j4 -f "Extern Win64.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -j4 -f "Traktor Win64.mak"
popd
