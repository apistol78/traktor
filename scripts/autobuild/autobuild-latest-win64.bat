@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x64.bat"

:: \hack Setup paths to ABI specific binaries.
@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x64\release
@set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x64

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-win64.bat"
cd "build\win64"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Win64.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Win64.mak"
popd
