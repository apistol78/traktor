@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-android.bat"
cd "build\android"
nmake -f "Extern Android.mak"
nmake -f "Traktor Android.mak"
popd
