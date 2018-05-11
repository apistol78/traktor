@echo off

set CONFIG=%*
if "%CONFIG%" == "" ( set CONFIG=all )

:: Setup our build environment.
call "%~dp0..\config.bat"
call "%~dp0..\config-vs-legacy.bat" 12.0

:: Rebuild entire solution.
pushd "%TRAKTOR_HOME%\build\ps3"
msbuild "Traktor Ps3.sln" /nologo /maxcpucount /p:BuildInParallel=true /p:Configuration=%CONFIG% /p:Platform="PS3"
popd
