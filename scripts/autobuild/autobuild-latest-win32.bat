@echo off

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: \hack Setup paths to ABI specific binaries.
@set FBX_SDK_LIBPATH=%FBX_SDK%\lib\vs2015\x86\release
@set P4_SDK=%TRAKTOR_HOME%\3rdp\p4api-2016.1.1350954.BETA-vs2015_dyn_x86

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%"
call "scripts\build-projects-make-win32.bat"
cd "build\win32"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Extern Win32.mak"
%TRAKTOR_HOME%\3rdp\jom\jom -f "Traktor Win32.mak"
popd

