@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x64.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win64"
::%TRAKTOR_HOME%\3rdp\jom\jom /J 1 /L /F "Extern Win64.mak" %CONFIG%
::%TRAKTOR_HOME%\3rdp\jom\jom /J 1 /L /F "Traktor Win64.mak" %CONFIG%

msbuild "Extern Win64.sln" /nologo /maxcpucount /p:Configuration=%CONFIG%
msbuild "Traktor Win64.sln" /nologo /maxcpucount /p:Configuration=%CONFIG%

popd
