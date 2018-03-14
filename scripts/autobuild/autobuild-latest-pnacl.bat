@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-pnacl.bat"
cd "build\pnacl"
nmake -f "Extern PNaCl.mak"
nmake -f "Traktor PNaCl.mak"
popd

