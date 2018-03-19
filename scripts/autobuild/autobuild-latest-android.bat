@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-android.bat"
cd "build\android"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Android.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Android.mak"
popd
