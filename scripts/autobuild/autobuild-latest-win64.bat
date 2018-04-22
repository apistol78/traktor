@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-x64.bat"

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\win64"
msbuild "Extern Win64.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="x64"
msbuild "Traktor Win64.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="x64"
popd
