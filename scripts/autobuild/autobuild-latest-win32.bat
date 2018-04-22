@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x86.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win32"
msbuild "Extern Win32.sln" /nologo /maxcpucount /p:Configuration=%CONFIG%
msbuild "Traktor Win32.sln" /nologo /maxcpucount /p:Configuration=%CONFIG%
popd

