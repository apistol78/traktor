@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\vsenv-legacy.bat" 14.0

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\ps4"
msbuild "Extern Ps4.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="ORBIS"
msbuild "Traktor Ps4.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="ORBIS"
popd

